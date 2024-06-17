#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "../include/worker.h"
#include "../include/controller.h"
#include "../include/buffer.h"

extern char* outputFile;
extern int concurrency;
extern shared_buffer_t request_buffer;
extern int bufSize;
//Worker threads should be made by main function according to argv[3] at start
char** splitJob(char* string) {
    char** args;
    char* token;
    int toks = 0;
    char* dup = strdup(string);


    token = strtok(dup," ");
    while(token) {
        toks++;
        token = strtok(NULL," ");
    }

    free(dup);

    args = malloc((toks+1)*sizeof(char*));

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
    job* toExec = bufferRemove(&request_buffer,bufSize);
    pid_t pid = fork();
    char* outputFile = NULL;
    char pidch[15];
    sprintf(pidch,"%d",getpid());
    outputFile = malloc(strlen(pidch)+strlen(".output")+1);
    sprintf(outputFile,"%s.output",pidch);
    if(pid == -1) {
        perror("Failed to fork");
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
    } else {
        int status;
        waitpid(pid,&status,0);
        sprintf(pidch,"%d",pid);
        sprintf(outputFile,"%s.output",pidch);
        int file = open(outputFile, O_RDONLY,0644);
        if(!file) {
            printf("Error opening file.\n");
            return NULL;
        }
        close(file);

    }
    return NULL;
}
