#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include "../include/worker.h"
#include "../include/controller.h"
#include "../include/buffer.h"

extern char* outputFile;
extern shared_buffer_t request_buffer;
extern pthread_cond_t concurrency;
extern int conLevel;
extern int bufSize;
extern pthread_mutex_t con_mutex;
int activeWorkers = 0;
//Worker threads should be made by main function according to argv[3] at start

char** splitJob(char* string) {
    char** args;
    char* token;
    int toks = 0;
    char* dup = strdup(string);


    //count amount of arguments in the command
    token = strtok(dup," ");
    while(token) {
        toks++;
        token = strtok(NULL," ");
    }

    free(dup);

    //allocate an array of arguments
    args = malloc((toks+1)*sizeof(char*));

    //add the arguments to the array
    int i = 0;
    token = strtok(string," ");
    while(token) {
        args[i] = strdup(token);
        token = strtok(NULL," ");
        i++;
    }
    args[i] = NULL;

    return args;
}

void* worker(void * arg) {
    while(1) {
        pthread_mutex_lock(&con_mutex);

        while(activeWorkers >= conLevel) {
            pthread_cond_wait(&concurrency,&con_mutex);
        }

        job* toExec = bufferRemove(&request_buffer,bufSize);

        activeWorkers++;
        // printf("ACTIVE : %d | CONLEVEL: %d\n",activeWorkers,conLevel);
        pthread_mutex_unlock(&con_mutex);


        pid_t pid = fork();

        char* outputFile;
        char pidch[15];
        
        sprintf(pidch,"%d",getpid());
        outputFile = malloc(strlen(pidch)+strlen(".output")+1);
        sprintf(outputFile,"%s.output",pidch);
        
        if(pid == -1) {
        
            perror("Failed to fork");
            activeWorkers--;
            free(outputFile);
            return NULL;
            
        } else if(pid == 0) {
            if(outputFile) {

                int file = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
                if(!file) {
                    printf("Error opening file.\n");
                    return NULL;
                }
                if (dup2(file, STDOUT_FILENO) < 0) {
                    perror("Error redirecting stdout");
                    close(file);
                    exit(EXIT_FAILURE);
                }
                close(file);
                free(outputFile);
    
            } else {
    
                fprintf(stderr,"Output file not given\n");
                free(outputFile);
                return NULL;

            }
    
            char** arguments = splitJob(toExec->job);
            execvp(arguments[0],arguments);
            printf("EXECVP FAILED");
            // int i = 0;
            // while(arguments[i]) {
                // free(arguments[i]);
                // i++;
            // }
            // free(arguments);
    
        } else {
    
            int status;
            waitpid(pid,&status,0);
    
            sprintf(pidch,"%d",pid);
            sprintf(outputFile,"%s.output",pidch);
    
            int file = open(outputFile, O_RDONLY,0644);
    
            if(!file) {
                printf("Error opening file.\n");
                activeWorkers--;
                return NULL;
            }
    
            char buf[1024];
            memset(buf,0,sizeof(buf));
            ssize_t bytes_read,bytes_written;
            //if there's data to read from file, then send that data over to the socket
            while((bytes_read = read(file,buf,sizeof(buf))) > 0) { 
                if((bytes_written = write(toExec->socketFd,buf,sizeof(buf))) == -1) {
                    perror("Error in write.\n");
                }
                memset(buf,0,sizeof(buf));
            }
    
            if(shutdown(toExec->socketFd, SHUT_WR) == -1) {
                perror("Shutdown failed.");
            } 
    
            close(file);
            remove(outputFile);
            close(toExec->socketFd);
            pthread_mutex_lock(&con_mutex);
            activeWorkers--;
            pthread_cond_signal(&concurrency);
            pthread_mutex_unlock(&con_mutex);
    
        }
    }
    return NULL;
}
