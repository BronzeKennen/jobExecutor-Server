#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}


int main(int argc, char** argv) { 
    if(argc < 4) {
        fprintf(stderr,"Usage: ./jobExecutorServer [portNum] [bufferSize] [threadPoolSize]\n");
    }
    int sockFd,newSockFd,portNum,concurrentConnections;

    
    struct sockaddr_in serverAddr,clientAddr;
    socklen_t clientLen;

    char *buffer;
    buffer = malloc(100); // will be replaced with actual dynamic allocation

    sockFd = socket(AF_INET,SOCK_STREAM,0);
    if(sockFd < 0) {
        error("Error opening socket.\n");
    }

    portNum = atoi(argv[1]);
    concurrentConnections = atoi(argv[1]);

    bzero((char*) &serverAddr,sizeof(serverAddr)); //Cleans up buffer or something
    serverAddr.sin_family = AF_INET; 
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(portNum);

    if(bind(sockFd,(struct sockaddr*) &serverAddr,sizeof(serverAddr)) < 0) {
        error("Binding Failed.");

    }

    listen(sockFd , concurrentConnections);
    clientLen = sizeof(clientAddr);

    newSockFd = accept(sockFd,(struct sockaddr*) &clientAddr, &clientLen);
    if(newSockFd < 0) {
        error("Error on accept function\n");
    }
    printf("Awaiting Client...\n");
    while(1) {
        int n = read(newSockFd, buffer, 100);
        if(n < 0) {
            error("Error on read");
        }
        printf("Client: %s\n",buffer);
        bzero(buffer,100);
    }
    close(newSockFd);
    close(sockFd);
    return 0;
}