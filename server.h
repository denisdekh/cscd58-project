#include "common.h"
#include "time.h"

#ifndef SERVER_H
#define SERVER_H

#define MAX_PENDING 5

/*
 Represents a user
*/
struct user {
    char username[MAX_LINE];    // username of user
    char password[MAX_LINE];    // password of user

    struct user *next;          // next user in user list
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

#endif