#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#ifndef NET_UTILS_H
#define NET_UTILS_H

#define assert(x, msg) if (!(x)) { fprintf(stderr, "%s", msg); exit(EXIT_FAILURE); }

#define MAX_REQUEST 4096    // maximum # of bytes for request
#define MAX_LINES 50        // maximum # of lines for request
#define MAX_LINE 1024        // maximum # of bytes per line

/*
 D58P: D58 Protocol definitions
*/
#define D58P_MESSAGE_STRING_REQ "D58P /Message"
#define D58P_MESSAGE_STRING_RES "D58P \\Message"
#define D58P_GET_MESSAGE_STRING_REQ "D58P /Get Message"
#define D58P_GET_MESSAGE_STRING_RES "D58P \\Get Message"
#define D58P_USER_STRING_REQ "D58P /User"
#define D58P_USER_STRING_RES "D58P \\User"
#define D58P_ERROR_STRING "D58P \\Error"
#define D58P_GET_KEY_REQ "D58P /Get Key"
#define D58P_GET_KEY_RES "D58P \\Get Key"

#define D58P_ACK_STRING "D58P /Acknowledge"

enum D58P_ResponseCode {
    D58P_OK=200,
    D58P_CREATED=201,
    D58P_BAD_REQUEST=400,
    D58P_UNAUTHORIZED=401,
    D58P_NOT_FOUND=404
};

struct D58P_auth {
    char username[MAX_LINE];         // type of request
    int user_len;                       // length of message
    
    char password[MAX_LINE];         // type of request
    int password_len;                   // length of message

    char e[MAX_LINE];                // values for the public key
    char n[MAX_LINE];
};

struct D58P_message_data {
    char target_user[MAX_LINE];
    int target_user_len;
    
    char message[MAX_LINE];          // type of request
    int message_len;                    // length of message
};

/*
 Represents a D58P request or response
*/
struct D58P {
    char lines[MAX_LINES][MAX_LINE];
};

/* client / server connections */
int create_connection(struct sockaddr_in *sin);
int accept_connection(int sfd, struct sockaddr_in *sin);

/* Helpers to create D58P request data */
void create_message_request(struct D58P *req, struct D58P_auth *auth, struct D58P_message_data *data);
void create_user_request(struct D58P *req, struct D58P_auth *auth);
void create_get_messages_request(struct D58P *req, struct D58P_auth *auth);

/* Helpers to create D58P response data */
void create_response(struct D58P *res, char *type, enum D58P_ResponseCode code);
void create_get_message_response(struct D58P *res, enum D58P_ResponseCode code, char *from, char buf[MAX_LINE]);

/* Sending / Receiving D58P data */
int send_D58P_request(struct sockaddr_in *sin, struct D58P *req, struct D58P *res);
void send_D58P_response(int sfd, struct D58P *res);
void send_D58P_response_ack(int sfd);

/* Other utils */
void parse_D58P_buf(struct D58P *data, char buf[MAX_REQUEST]);
void dump_D58P(struct D58P *data);
int verify_acknowledgement(int sfd);

#endif