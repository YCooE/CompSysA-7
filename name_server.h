#include "csapp.h"

// User struct
struct User {
  char username[30];
  char password[30];
  char ip[30];
  char port[5];
  int status;
};

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

// Returns 1 if the user was found, returns 0 if not.
int get_user(char* username, struct User** save_user, struct User* userlist, int* users){
  struct User* ptr = userlist;
  for (int i = 0; i < *users; i++, ptr++) {
    // compare strings
    if(strcmp(username, ptr->username) == 0){
      // Set save_user to current if strings are the same. Return 1
      *save_user = ptr;
      return 1;
    }
  }
  return 0;
}

// Commands:
void login(int* connfd, char* input, struct User* userlist, int* users, struct User** this_user, int* logout) {
  char* token = strtok(input, " ");

  // Get first argument which is username
  token = strtok(NULL, " ");
  strcpy(input, token);

  if (get_user(input, this_user, userlist, users) == 1) {
    // Get second argument which is password
    token = strtok(NULL, " ");
    strcpy(input, token);
    // If password is correct allow connection
    if (strcmp(input, (*this_user)->password) == 0) {
      // If user is already online:
      if ((*this_user)->status == 0) {
        // Set IP and port
        strcpy((*this_user)->ip, strtok(NULL, " "));
        strcpy((*this_user)->port, strtok(NULL, "\n"));
        (*this_user)->status = 1;
        strcpy(input, "You are now logged in.\n");
        Rio_writen(*connfd, input, strlen(input));
      }
      else {
        strcpy(input, "Error: User is already online!\n");
        Rio_writen(*connfd, input, strlen(input));
        *logout = 1;
      }
    }
    else {
      strcpy(input, "Error: Wrong password!\n");
      Rio_writen(*connfd, input, strlen(input));
      *logout = 1;
    }
  }
  else {
    strcpy(input, "Error: User not found!\n");
    Rio_writen(*connfd, input, strlen(input));
    *logout = 1;
  }
}

// Ends loop
void logoutCommand(int* connfd, char* input, int* logout, struct User* this_user) {
  strcpy(input, "You are now logged out.\n");
  Rio_writen(*connfd, input, strlen(input));
  this_user->status = 0;
  *logout = 1; // End while loop, and close connection file descriptor.
  return;
}

// Returned if client request isn't valid.
void notValidCommand(int* connfd, char* input){
  strcpy(input, "Not a valid server command.\n");
  Rio_writen(*connfd, input, strlen(input));
}

// Lookup specific user IP and Port, or all users IPs and Ports
void lookup(int* connfd, char* input, struct User* userlist, int* users) {
  if (arg_count(input) == 1) {
    char buffer[30];
    char* token = strtok(input, " ");

    // Get first argument which is username
    token = strtok(NULL, "\n"); // Removes newline
    strcpy(buffer, token);

    struct User* find_user = userlist;


    // If user exists output relevant info:
    if (get_user(buffer, &find_user, userlist, users) == 1) {
      // If user is online output status IP and Port
      if(find_user->status == 1) {
        sprintf(input, "\nThe user %s is online\nIP: %s\nPort: %s \n\n", find_user->username, find_user->ip, find_user->port);
        Rio_writen(*connfd, input, strlen(input));
      }
      // If user is offline output status
      else {
        sprintf(input, "The user %s is offline\n", find_user->username);
        Rio_writen(*connfd, input, strlen(input));
      }
    }
    else {
      sprintf(input, "Error: The user %s does not exist\n", buffer);
      Rio_writen(*connfd, input, strlen(input));
    }
  }
  else if (arg_count(input) == 0) {
    int usersonline = 0;
    struct User* ptr = userlist;
    for (int i = 0; i < *users; i++, ptr++) {
      if(ptr->status == 1){
        usersonline++;
      }
    }
    char buffer[MAXLINE];
    if (usersonline == 1) {
      sprintf(buffer, "\n%d user online. The list follows:\n", usersonline);
    }
    else {
      sprintf(buffer, "\n%d users online. The list follows:\n", usersonline);
    }

    strcpy(input, buffer);
    ptr = userlist;
    for (int i = 0; i < *users; i++, ptr++) {
      if(ptr->status == 1){
        sprintf(buffer, "Nick: %s\nIP: %s\nPort: %s \n", ptr->username, ptr->ip, ptr->port);
        strcat(input, buffer);
      }
    }
    strcat(input, "\n");
    Rio_writen(*connfd, input, strlen(input));
  }
  else {
    strcpy(input, "Usage: /lookup or /lookup <USERNAME>\n");
    Rio_writen(*connfd, input, strlen(input));
  }
}

// The name_server loop:
void nameserverLoop(int* connfd, struct User* userlist, int* users) {
  char buffer[MAXLINE];
  rio_t rio;

  Rio_readinitb(&rio, *connfd);

  // Connected until client types /logout
  int logout = 0;

  // The user being used in this connection
  struct User* this_user = userlist;

  // Keep running until logout
  while(!logout) {
    Rio_readlineb(&rio, buffer, MAXLINE);
    // Check if the connection is dead
    if (recv(*connfd, buffer, sizeof(buffer), MSG_PEEK | MSG_DONTWAIT) == 0) {
        this_user->status = 0;
        return;
    }
    printf("Received the following input: %s", buffer);
    char* input = buffer;

    if (partOfArray(input, 0, 5, "/login")) {
      login(connfd, input, userlist, users, &this_user, &logout);
    }
    else if (partOfArray(input, 0, 6, "/logout")) {
      logoutCommand(connfd, input, &logout, this_user);
    }
    else if (partOfArray(input, 0, 6, "/lookup")) {
      lookup(connfd, input, userlist, users);
    }
    else {
      notValidCommand(connfd, input);
    }
  }
}
