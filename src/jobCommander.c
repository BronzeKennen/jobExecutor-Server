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
    int portNum,sockFd,status;
    struct addrinfo hints, *res, *p;
    int cmdSize = 0; //Needed to allocate for string and send size to server
    for(int i = 3; i < argc; i++) {
        cmdSize += strlen(argv[i]);
    }

    char* command = malloc(cmdSize+1); //parse Command

    if(strncmp(argv[3],"setConcurrency",14) == 0) {
        if(strlen(argv[4]) > 8) {
            printf("Invalid concurrency\n");
            exit(1);
        }
        int con = atoi(argv[4]);
        char buf[20];
        sprintf(buf,"setConcurrency %d",con);
        strncpy(command,buf,20);

    } else if (strncmp(argv[3],"issueJob",8) == 0) {
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
        char buf[20];
        snprintf(buf,20,"stop %s",argv[4]);
        strcpy(command,buf);

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
    
    char buffer[cmdSize];
    memset(buffer,0,cmdSize);
    memcpy(buffer,command,cmdSize);
    printf("command : %s\n",command);
    printf("command size : %d\n",cmdSize);
    int n = write(sockFd,buffer,cmdSize);
    if(n < 0) {
        error("Error on write");
    }
    memset(buffer,0,cmdSize);
    n = read(sockFd, buffer,cmdSize);
    if(n < 0 ) {
        error("error on reading");
    }
    printf("RESPONSE : %s \n",buffer);
    close(sockFd);
    return 0;
}