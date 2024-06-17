#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>

void error(const char *msg) {
    perror(msg);
    exit(1);
}

int serverInit() {

    return 0;
}

int main(int argc, char** argv) {
    if(argc < 4) {
        fprintf(stderr,"Usage: ./jobCommander [serverName] [portNum] [jobCommanderInputCommand]\n");
        exit(1);
    }
    int handshake,portNum,sockFd,status;
    struct addrinfo hints, *res, *p;
    int cmdSize = 0; //Needed to allocate for string and send size to server
    for(int i = 3; i < argc; i++) {
        cmdSize += strlen(argv[i]);
        cmdSize++;
    }

    char command[cmdSize+1];
    memset(command,0,cmdSize+1);

    if(strncmp(argv[3],"setConcurrency",14) == 0) {
        if(strlen(argv[4]) > 8) {
            printf("Invalid concurrency\n");
            exit(1);
        }
        int con = atoi(argv[4]);
        if(con < 1) {
            printf("Invalid concurrency\n");
            exit(1);
        }
        char buf[cmdSize];
        snprintf(buf,cmdSize,"setConcurrency %d",con);
        strncpy(command,buf,cmdSize);

    } else if (strncmp(argv[3],"issueJob",8) == 0) {
        handshake = 1; //Send size first, get ACK, send actual data
        int cur = 0;
        for(int i = 3; i < argc; i++) { //and all of its arguments...
            strcpy(command+cur,argv[i]);
            cur += strlen(argv[i]);
            command[cur++] = ' ';
            cmdSize+=1;
        }

        command[cur-1] = '\0';

    } else if (strncmp(argv[3],"stop",4) == 0)  {
        if(strlen(argv[4]) > 10) {
            fprintf(stderr,"Invalid jobId.\n");
            exit(1);
        }
        char buf[cmdSize];
        snprintf(buf,cmdSize,"stop %s",argv[4]);
        strncpy(command,buf,cmdSize);

    } else if (strncmp(argv[3],"exit",4) == 0) {
        strcpy(command,"exit"); //prints some random character at the end in the exit function specifically
        command[4] = '\0';

    } else if(strncmp(argv[3],"poll",4) == 0) {
        strcpy(command,"poll");
        command[4] = '\0';
    } else {
        printf("Invalid command");
        exit(1);
    }


    portNum = atoi(argv[2]);
    memset(&hints,0,sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    char portStr[6];
    snprintf(portStr,sizeof(portStr),"%d",portNum);

    if((status = getaddrinfo(argv[1],portStr,&hints,&res)) != 0) {
        fprintf(stderr,"getaddrinfo %s\n",gai_strerror(status));
        exit(1);
    }

    for(p = res; p != NULL; p = p->ai_next) {
        if((sockFd = socket(p->ai_family,p->ai_socktype, p->ai_protocol)) == -1) { //try every address or something?
            perror("Socket");
            continue;
        }

        if (connect(sockFd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockFd);
            perror("connect");
            continue;
        }

        break;  // Successfully connected
    }
    if (p == NULL) {
        fprintf(stderr, "Failed to connect to any server.\n");
        exit(1);
    }
    
    char buffer[cmdSize+1];
    memset(buffer,0,cmdSize+1);
    memcpy(buffer,command,cmdSize+1); 

    printf("Sending: %s\n",buffer);
    int n = write(sockFd,buffer,cmdSize); //Send command
    if(!handshake) {
        if (shutdown(sockFd, SHUT_WR) == -1) {
            perror("shutdown failed");
        }
    }
    if(n < 0) {
        error("Error on write");
    }

    char respBuf[128]; //I know responses are fixed size so static buffer is okay
    memset(respBuf,0,128);
    n = read(sockFd,respBuf,128);
    if(n < 0 ) {
        error("error on reading");
    }
    printf("Response : %s\n",respBuf);
    //if issuejob is the command an exchange of dynamic size has to happen
    if(handshake) { 
        char sizeBuf[10];
        sprintf(sizeBuf,"%d",cmdSize);
        memcpy(buffer,sizeBuf,cmdSize);
        n = write(sockFd,buffer,cmdSize);
        if(n < 0) {
            error("Error on write");
        }
        memset(buffer,0,cmdSize);
        n = read(sockFd, buffer,cmdSize);
        if(n < 0 ) {
            error("error on reading");
        }
        int comp = atoi(buffer);

        if(comp == cmdSize) { //We are good to go
            memset(buffer,0,cmdSize);
            memcpy(buffer,command,cmdSize);
            n = write(sockFd,buffer,cmdSize);
            char resp[
                        strlen("job")
                        +strlen("submitted")
                        +cmdSize
                        +strlen("job_")
                        +5 //I have to change that according to jobId
                    ];
            memset(resp,0,sizeof(resp));
            n = read(sockFd, resp,sizeof(resp));
            if(n < 0 ) {
                error("error on reading");
            }
            printf("Response :\n %s \n",resp);
        }
        //Read output of execution
        char execBuf[1024];
        memset(execBuf,0,sizeof(respBuf));
        //read command in chunks of 1024
        while((n=read(sockFd,execBuf,sizeof(execBuf))) > 0) {
            execBuf[n] = '\0';
            printf("\n=====READING EXECUTION OUTPUT=====\n");
            printf("%s",execBuf);
            printf("\n====END OF EXECUTION OUTPUT=====\n");
        }
        shutdown(sockFd,SHUT_RDWR);

    }
    close(sockFd);
    return 0;
}