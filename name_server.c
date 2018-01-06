#include <stdio.h>
#include "name_server.h"

#define ARGNUM 0 // TODO: Put the number of you want to take

void *thread(void *vargp);

struct thread_args {
    struct User *userlist;
    int users;
    int *connfd;
};

int main(int argc, char**argv) {
    // Take 0 arguments
    if (argc != ARGNUM + 1) {
        printf("%s expects %d arguments.\n", (argv[0]+2), ARGNUM);
        return(0);
    }

    // User list
    int users = 3;
    struct User userlist[3];

    struct thread_args t_args;
    t_args.users = users;

    // Hard coded users for testing
    strcpy(userlist[0].username, "test1");
    strcpy(userlist[0].password, "password");
    userlist[0].status = 0;

    strcpy(userlist[1].username, "test2");
    strcpy(userlist[1].password, "password");
    userlist[1].status = 0;

    strcpy(userlist[2].username, "u");
    strcpy(userlist[2].password, "p");
    userlist[2].status = 0;

    t_args.userlist = userlist;
    // Server port:
    char port[] = "1776";

    // Listen socket
    int listenfd;
    int *connfd; // Only one connected file descriptor, make it so there can be multiple.

    // Socket
    socklen_t clientlen;
    struct sockaddr_storage clientaddr; // Works for any protocol

    // Client IP and Port
    char client_hostname[MAXLINE], client_port[MAXLINE];

    listenfd = Open_listenfd(port);
    pthread_t tid;
    while (1) {
      // Size of sockaddr_storage:
      clientlen = sizeof(struct sockaddr_storage);
      connfd = Malloc(sizeof(int));
      // Wait until connection requests
      *connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);
      // client info:
      Getnameinfo((SA *) &clientaddr, clientlen, client_hostname, MAXLINE,
                  client_port, MAXLINE, 0);
      printf("Connected to (%s:%s)\n", client_hostname, client_port);
      t_args.connfd = connfd;
      Pthread_create(&tid, NULL, &thread, &t_args);
    }
}

// Thread routine
void *thread(void *vargp)
{
    struct thread_args *var = vargp;
    int connfd = *((int *)var->connfd);
    Pthread_detach(pthread_self());
    Free(((int *)var->connfd));
    nameserverLoop(&connfd, &var->userlist[0], &var->users);
    Close(connfd);
    return NULL;
}
