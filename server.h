#include "common.h"
#include "arpa/inet.h"
#include "time.h"

#ifndef SERVER_H
#define SERVER_H

#define MAX_PENDING 10

/*
 Represents a user
*/
struct user {
    char username[MAX_LINE];    // username of user
    char password[MAX_LINE];    // password of user

    char e[MAX_LINE];           // public key
    char n[MAX_LINE];           // public key

    struct user *next;          // next user in user list

    char e[KEY_LENGTH];         // public key values
    char n[KEY_LENGTH];
};

struct message {
    char buf[MAX_LINE];         // contents of message

    time_t time;                // time the server received the message
                                // to ensure messages arrive in order

    struct user *from;          // sender of message
    struct user *to;            // destination of message
    struct message *next;       // next message in message list
    struct message *prev;       // prev message in message list.
                                // prev of head points to tail
};

/*
 List element for a list of threads who are currently handling D58P /Get Message requests.

 This is implemented so when a user changes their connection / loses connection
 messages are routed properly.
*/
struct server_thread {
    pthread_t tid;              // tid of this thread
    time_t started;             // time this thread started
    int client_socket;          // client socket for this connection
    struct user *user;          // user struct this thread is getting messages for

    int connection_switched;    // flag to determine if connection switched

    struct server_thread *next; // next server_thread in list
};

#endif