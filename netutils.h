#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifndef NET_UTILS_H
#define NET_UTILS_H

#define assert(x, msg) if (!(x)) { fprintf(stderr, "%s", msg); exit(EXIT_FAILURE); }
#define MAX_REQUEST 4096    // maximum # of bytes for request


/*
 D58P: D58 Protocol definitions
*/
#define D58P_MESSAGE_STRING "D58P /Message"
#define D58P_USER_STRING "D58P /User"

enum D58P_RequestType {
    D58P_Type_Message,      // message to user
    D58P_Type_User          // login/register
};

struct D58P_auth {
    char username[MAX_REQUEST];         // type of request
    int user_len;               // length of message
    
    char password[MAX_REQUEST];         // type of request
    int password_len;       // length of message
};


struct D58P_message_data {
    char target_user[MAX_REQUEST];
    int target_user_len;
    
    char message[MAX_REQUEST];          // type of request
    int message_len;        // length of message
};

struct D58P_request {
    char buf[MAX_REQUEST];  // request data
    int len;                // request length
};


int create_connection(struct sockaddr_in *sin);
int accept_connection(int sfd, struct sockaddr_in *sin);

void create_message_request(struct D58P_request *req, struct D58P_auth *auth, struct D58P_message_data *data);
void create_user_request(struct D58P_request *req, struct D58P_auth *auth);


#endif