#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <pthread.h>
#include "../include/controller.h"
#include "../include/buffer.h"

int conLevel = 1;

extern shared_buffer_t request_buffer;
extern int bufSize;
extern pthread_mutex_t con_mutex;
extern pthread_cond_t concurrency;

void adjustConcurrency(int newCon) {
    pthread_mutex_lock(&con_mutex);
    conLevel = newCon;

    pthread_cond_broadcast(&concurrency);

    pthread_mutex_unlock(&con_mutex);
}

void* controller(void* arg) {
    int newSockFd = *(int*)arg;
    free(arg);

    printf("client connected\n");
    //Initial buffer, if issueJob is given, another buffer is created with dynamic size
    char buffer[256]; 
    while(1) { //implement logic to handle commands
        //implement reading size dynamically
        /*
        Commands:
            issueJob [command (arguments)]
            setConcurrency [num]
            stop [jobId]
            poll
            exit
        */
        int n = read(newSockFd, buffer, 256);
        if(n < 0) {
            perror("Error on read");
            exit(1);
        } else if (n == 0) {
            printf("Client disconnected\n");
            close(newSockFd);
            return NULL;
        }
        if(strncmp(buffer,"issueJob",8) == 0) { //Command issueJob
            memset(buffer,0,256);
            strcpy(buffer,"ACK\n"); //Send confirmation that command was received
            write(newSockFd,buffer,3);
            memset(buffer,0,256);
            n = read(newSockFd,buffer,256); //read size
            if(n < 0) {
                perror("Error on read");
                exit(1);
            }
            int size = atoi(buffer);
            char newBuf[size+1]; //Create buffer of needed size
            memset(newBuf,0,size);

            memset(buffer,0,256);
            char recSize[8];
            memset(recSize,0,8);
            sprintf(recSize,"%d",size);
            strcpy(buffer,recSize); //Send confirmation that size was given
            
            write(newSockFd,buffer,3);
            memset(newBuf,0,size+1);
            n = read(newSockFd,newBuf,size); //Read the contents  
            
            if(n < 0) {
                perror("Error on read");
                exit(1);
            }
            job *jobTriplet = malloc(sizeof(job));
            jobTriplet->socketFd = newSockFd;
            jobTriplet->job = malloc(strlen(newBuf));
            strcpy(jobTriplet->job,newBuf+9);
            bufferAdd(&request_buffer,jobTriplet,bufSize);
            //Logic to add to buffer
            memset(buffer,0,size+1);
            char resp[
                        strlen("job")
                        +strlen("submitted")
                        +strlen(jobTriplet->job)
                        +strlen("job_")
                        +2 //I have to change that according to jobId
                    ];
            sprintf(resp,"JOB <job_%d,%s> SUBMITTED\n",jobTriplet->id,jobTriplet->job);
                
            memcpy(buffer,resp,strlen(resp));
            write(newSockFd,buffer,strlen(resp));

        } else if(strncmp(buffer,"stop",4) == 0) {
            int id = atoi(buffer+9);
            char temp[10];
            memset(temp,0,10);
            strcpy(temp,buffer+9);
            int size = strlen(temp);
            job jobId = {0,id,NULL};
            job* another = bufferRemoveOnFind(&request_buffer,&jobId,bufSize);

            char temp2[strlen("JOB  NOT FOUND")+size];
            memset(temp2,0,sizeof(temp2));
            if(another) 
                sprintf(temp2,"JOB %d STOPPED\n",id);
            else
                sprintf(temp2,"JOB %d NOT FOUND\n",id);

            write(newSockFd,temp2,strlen(temp2));
            if (shutdown(newSockFd, SHUT_WR) == -1) {
                perror("shutdown failed");
            }


        } else if(strncmp(buffer,"poll",4) == 0) {
            bufferPrint(&request_buffer,bufSize);
            memset(buffer,0,256);
            strcpy(buffer,"No jobs to print\n\0");
            write(newSockFd,buffer,strlen(buffer));
            if (shutdown(newSockFd, SHUT_WR) == -1) {
                perror("shutdown failed");
            }

        } else if(strncmp(buffer,"setConcurrency",14) == 0) {
            int con = atoi(buffer+15);
            adjustConcurrency(con);

        } else if(strncmp(buffer,"exit",4) == 0) {
            //Implement logic to reply with "Server terminated before execution"
            //Need to check when other processes finished.
            //Need to send message to queued items that server terminated.
            printf("Server is terminated.\n");
            memset(buffer,0,256);
            strcpy(buffer,"Server will terminate when running jobs finish...\n");
            write(newSockFd,buffer,strlen(buffer));

        }
        printf("Client : %s\n",buffer); //Garbage values if issueJob is used
    }
    return NULL;
}