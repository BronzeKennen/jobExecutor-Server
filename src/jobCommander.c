#include <stdlib.h> 
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

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
    // int status,portNum;
    struct hostent *server;
    struct sockaddr_in serverAddr;


    // char* buffer;

    // buffer = malloc(100);
    server = gethostbyname(argv[1]);
    if(server == NULL) {
        fprintf(stderr,"Error no such host\n");
        exit(1);
    }
    bzero((char*) &serverAddr,sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    bcopy((char*) server->h_addr_list,(char*) &serverAddr.sin_addr.s_addr,server->h_length);

    

    return 0;
}