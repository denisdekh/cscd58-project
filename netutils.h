#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef NET_UTILS_H
#define NET_UTILS_H

int create_connection(struct sockaddr_in *sin);
int accept_connection(int sfd, struct sockaddr_in *sin);

#endif