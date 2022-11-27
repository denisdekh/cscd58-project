#include "netutils.h"
#include <string.h>

#define SERVER_PORT 6777
#define MAX_PENDING 5
#define MAX_LINE 256

/* global variables */
int sfd;                        // fd for socket
struct sockaddr_in s_addr;      // address struct

/*
 server loop
 return 0: successful server loop
 return 1: terminate loop
*/
int handle_connection(int client_socket)
{
    int len;
    char buf[MAX_LINE];

    while (len = recv(client_socket, buf, sizeof(buf), 0)) {
        // create a buffer to reply to client.
        // length of buffer is length of received message + length of date string +1 to add a \n in the reply
        char reply_buf[MAX_LINE] = "Reply from server\n";

        send(client_socket, reply_buf, MAX_LINE, 0); // send back to client

        fputs(buf, stdout); // output received message from client to stdout
    }
}

int server_loop()
{
    int client_socket = accept_connection(sfd, &s_addr);

    handle_connection(client_socket);
    close(client_socket);
}


int main()
{    
    // setup address struct
    bzero((char *)&s_addr, sizeof(s_addr));
    s_addr.sin_family = AF_INET;
    s_addr.sin_addr.s_addr = INADDR_ANY;
    s_addr.sin_port = htons(SERVER_PORT);
    
    // create socket
    if ((sfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("server: socket");
        exit(1);
    }

    // make socket reusable
    int option = 1;
    setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));

    // bind socket to port
    if ((bind(sfd, (struct sockaddr *)&s_addr, sizeof(s_addr))) < 0) {
        perror("server: could not bind");
        exit(1);
    }

    // listen for connections
    listen(sfd, MAX_PENDING);

    // main server loop
    while(1) {
        server_loop();
    }

    close(sfd); // should never get here though
}