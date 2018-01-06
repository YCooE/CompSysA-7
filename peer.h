#include "csapp.h"

// Get number of arguments:
int arg_count(char* input) {
  int args = -1; // We don't want to count the command itself
  char *arg;
  // Copy so that we do not modify data
  char copy[MAXLINE];
  strcpy(copy, input);

  // Count arguments
  arg = strtok(copy, " ");

  while(arg != NULL ) {
     args++;
     arg = strtok(NULL, " ");
  }

  return args;
}

// Takes a string and the range specified by fist and last
// and then compares to the string compare if equal return
// true, else return false.
int partOfArray(char* input, int first, int last, char* compare){
  int retval = 1;
  int j = 0;
  for(int i = first; i <= last; i++) {
    if (input[i] != compare[j]){
      retval = 0;
    }
    j++;
  }

  return retval;
}

// Commands:
// Login to server if not already connected.
void login(int* client, char* input, char* host, char* port, int* status, rio_t* rio) {
  if (*status == 0) {
    // Must contain 4 arguments: login <USERNAME> <PASSWORD> <IP> <PORT>
    if (arg_count(input) != 4) {
      printf("Usage: /login <USERNAME> <PASSWORD> <IP> <PORT>\n");
      return;
    }
    // Establish connection
    *client = Open_clientfd(host, port);
    *status = 1;
    Rio_readinitb(rio, *client);
    Rio_writen(*client, input, strlen(input)); // Pass login info to server
    // Make it so that if the server returns error kill connection :)
    Rio_readlineb(rio, input, MAXLINE); // Type what is returned
    if (partOfArray(input, 0, 4, "Error")){
      Fputs(input, stdout);
      Close(*client);
      *status = 0;
    }
    else {
      Fputs(input, stdout);
    }

  }
  else {
    printf("Already connected to server\n");
  }
}

void logout(int* client, char* input, int* status, rio_t* rio) {
  if (*status == 1) {
    Rio_writen(*client, input, strlen(input)); // Pass command to server
    Rio_readlineb(rio, input, MAXLINE); // Print "you have been logged out"
    if (partOfArray(input, 0, 6, "/logout")) {
      strcpy(input, "Could not connect to server, you have been logged out\n");
      Fputs(input, stdout);
    }
    else {
      Fputs(input, stdout);
    }
    Close(*client);
    *status = 0;
  }
  else {
    printf("Not connected to server\n");
  }
}

void exitCommand(int* status) {
  if (*status == 0) {
    exit(1);
  }
  else {
    printf("Logout before exiting program\n");
  }
}

// Interprets the command, uses connection status to act correctly.
void interpret(char* input, int* status, int* client, char* host, char* port, rio_t* rio) {
  // If connected to server client should only interpret these commands
  if (partOfArray(input, 0, 5, "/login")) {
    login(client, input, host, port, status, rio);
  }
  else if (partOfArray(input, 0, 6, "/logout")) {
    logout(client, input, status, rio);
  }
  else if (partOfArray(input, 0, 4, "/exit")) {
    exitCommand(status);
  }
  else if (*status == 1) {
    // Send other commands to name_server
    Rio_writen(*client, input, strlen(input));
    Rio_readlineb(rio, input, MAXLINE);
    if (input[0] == '\n') {
      while (1) {
        Rio_readlineb(rio, input, MAXLINE);
        if (input[0] != '\n') {
          Fputs(input, stdout);
        }
        else {
          break;
        }
      }
    }
    else {
      Fputs(input, stdout);
    }
  }
  else {
    printf("Not a valid client command\n");
  }
}
