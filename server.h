#include "common.h"

#ifndef SERVER_H
#define SERVER_H

#define MAX_PENDING 5

struct user {
    char username[MAX_LINE];
    char password[MAX_LINE];

    struct user *next;
};

#endif