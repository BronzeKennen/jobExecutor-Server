#include <stdlib.h> 
#include <stdio.h>
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
        printf("Usage: ./jobCommander [serverName] [portNum] [jobCommanderInputCommand]\n");
        exit(1);
    }
    

    return 0;
}