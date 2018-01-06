#include <stdio.h>
#include "peer.h"

#define ARGNUM 0 // TODO: Put the number of you want to take


int main(int argc, char**argv) {
    if (argc != ARGNUM + 1) {
        printf("%s expects %d arguments.\n", (argv[0]+2), ARGNUM);
        return(0);
    }

    // Setup
    int clientfd;
    char* host;
    char* port;
    host = "127.0.0.1";
    port = "1776";
    rio_t rio;

    // Connection status 0 = not connected, 1 = connected
    int connectionStatus = 0;

    // Create a buffer for inputs, MAXLINE is a macro set in CSAPP.h
    char buffer[MAXLINE];

    // Let user type in input, and check for various
    while(Fgets(buffer, MAXLINE, stdin) != NULL) {
      interpret(buffer, &connectionStatus, &clientfd, host, port, &rio);
    }

    return 0;
}
