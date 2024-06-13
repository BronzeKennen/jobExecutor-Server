#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include "../include/controller.h"
#include <string.h>

void* controller(void* arg) {
    int newSockFd = *(int*)arg;

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
            exit(0);
        }
        if(strncmp(buffer,"issueJob",8) == 0) { //Command issueJob
            memset(buffer,0,256);
            strcpy(buffer,"ACK\n"); //Send confirmation that command was received
            write(newSockFd,buffer,3);
            printf("ACK\n");
            memset(buffer,0,256);
            n = read(newSockFd,buffer,256); //read size
            if(n < 0) {
                perror("Error on read");
                exit(1);
            }
            int size = atoi(buffer);
            printf("Size received: %d\n",size);
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
            printf("Client : %s\n",newBuf);

        } else if(strncmp(buffer,"stop",4) == 0) {

        } else if(strncmp(buffer,"poll",4) == 0) {

        } else if(strncmp(buffer,"setConcurrency",14) == 0) {

        } else if(strncmp(buffer,"exit",4) == 0) {

        }
        printf("Client : %s\n",buffer);
        memset(buffer,0,256);
        strcpy(buffer,"ACK\n");
        write(newSockFd,buffer,3);
    }
    return NULL;
}