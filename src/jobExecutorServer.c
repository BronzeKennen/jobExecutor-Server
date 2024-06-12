#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/wait.h>

int concurrency = 1;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void handleClient(int newSockFd) {
    char buffer[256];
    printf("client connected\n");
    while(1) {
        int n = read(newSockFd, buffer, 256);
        if(n < 0) {
            error("Error on read");
        } else if (n == 0) {
            printf("Client disconnected\n");
            close(newSockFd);
            exit(0);
        }
        printf("Client: %s\n",buffer);
        memset(buffer,0,256);
        strcpy(buffer,"ACK\n");
        write(newSockFd,buffer,5);
    }
}

int main(int argc, char** argv) { 
    if(argc < 4) {
        fprintf(stderr,"Usage: ./jobExecutorServer [portNum] [bufferSize] [threadPoolSize]\n");
    }
    int sockFd,newSockFd,portNum,concurrentConnections;

    
    struct sockaddr_in serverAddr,clientAddr;
    socklen_t clientLen;


    sockFd = socket(AF_INET,SOCK_STREAM,0);
    if(sockFd < 0) {
        error("Error opening socket.\n");
    }

    portNum = atoi(argv[1]);
    concurrentConnections = atoi(argv[2]);

    memset((char*) &serverAddr,0,sizeof(serverAddr)); //Cleans up buffer or something
    serverAddr.sin_family = AF_INET; 
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(portNum);

    if(bind(sockFd,(struct sockaddr*) &serverAddr,sizeof(serverAddr)) < 0) {
        error("Binding Failed.");

    }

    if(listen(sockFd , concurrentConnections)<0) {
        error("error on listen");
    }
    clientLen = sizeof(clientAddr);

    while(1) {
        newSockFd = accept(sockFd,(struct sockaddr*) &clientAddr, &clientLen); //if no client connects program hangs until someone connects
        if(newSockFd < 0) {
            error("Error on accept function\n");
            exit(1);
        } 
        
        if(fork() == 0) {
            close(sockFd);
            handleClient(newSockFd);
        } else {
            close(newSockFd);
            wait(NULL);
        }
    }
    close(newSockFd);
    close(sockFd);
    return 0;
}