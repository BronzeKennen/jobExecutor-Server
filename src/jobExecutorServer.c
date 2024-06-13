#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <pthread.h>
#include "../include/controller.h"
#include "../include/worker.h"
#include "../include/buffer.h"
/*
main => Create threadPoolSize worker threads on init
on accept => Create controller thread

Controller thread: 
    Assign tasks. if issueJob etc
        on issueJob => <jobId, job, clientSocket> => Return JOB <jobId,job> SUBMITTED
        on setConcurrency => change local var => Return CONCURRENCY SET AT N
        on stop => stop if exists => return Removed else return NOT FOUND
        on poll => print stuff
        on exit => terminate server (IF TASKS IN QUEUE => SERVER TERMINATED BEFORE EXECUTION)

Worker thread:
    Read from buffer if there's anything
    Run task
    Send output to client

    Condition. A worker thr ead is awake only when there's a task to complete
        => Check concurrency (if can run / cant run)
            => wait or something
            => Make a txt with the output of the program 
            => use dup2 to redirect stdout to file. 
            => Fork,exec
            => wait for child
            => on death parent sends output to client

Synchronization:
*/
int concurrency = 1;

void error(const char *msg) {
    perror(msg);
    exit(1);
}


int main(int argc, char** argv) { 
    if(argc < 4) {
        fprintf(stderr,"Usage: ./jobExecutorServer [portNum] [bufferSize] [threadPoolSize]\n");
    }
    int sockFd,newSockFd,portNum,bufSize,activeControllers;//activeWorkers;
    // int threadNum = atoi(argv[3]);

    // pthread_t workerThreads[threadNum]; //Concurrency basically
    // int workerThreadIds[threadNum];

    activeControllers = 0;
    // activeWorkers = 0;

    bufSize = atoi(argv[2]);

    pthread_t controllerThreads[bufSize];
    // char* buffer[bufSize]; 

    shared_buffer_t request_buffer = {
        .buffer = malloc(bufSize),
        .in = 0,
        .out = 0,
        .count = 0,
        .mutex = PTHREAD_MUTEX_INITIALIZER,
        .not_full = PTHREAD_COND_INITIALIZER,
        .not_empty = PTHREAD_COND_INITIALIZER
    };

    struct sockaddr_in serverAddr,clientAddr;
    socklen_t clientLen;
    // pthread_t *thread;


    sockFd = socket(AF_INET,SOCK_STREAM,0);
    if(sockFd < 0) {
        error("Error opening socket.\n");
    }

    portNum = atoi(argv[1]);

    memset((char*) &serverAddr,0,sizeof(serverAddr)); //Cleans up buffer or something
    serverAddr.sin_family = AF_INET; 
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(portNum);

    if(bind(sockFd,(struct sockaddr*) &serverAddr,sizeof(serverAddr)) < 0) {
        error("Binding Failed.");

    }

    if(listen(sockFd , bufSize)<0) {
        error("error on listen");
    }
    clientLen = sizeof(clientAddr);

    while(1) {
        newSockFd = accept(sockFd,(struct sockaddr*) &clientAddr, &clientLen); //if no client connects program hangs until someone connects
        if(newSockFd < 0) {
            error("Error on accept function");
            exit(1);
        } 
        
        if(pthread_create( //Create a controller thread
            &controllerThreads[activeControllers],
            NULL,
            controller,
            (void*)&newSockFd
        ) != 0) {
            error("Could not create controller thread");
        } 
    }
    close(newSockFd);
    close(sockFd);
    free(request_buffer.buffer);
    return 0;
}