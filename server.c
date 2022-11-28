#include "server.h"

/* global variables */
int sfd;                        // fd for socket
struct sockaddr_in s_addr;      // address struct


void user_handler(char buf[MAX_LINE], int len)
{
    /*
     TODO: should allow requesting user to login/register

     If the <user> exists in the server and <password> is valid then login as this user.
     If the <user> does not exist in the server then register a new user with <user>/<password> and login

     We can split this into a separate register & login if you want
    */

    printf("server: user handler\n");
}

/*
 regular_handler

 Handles logic to be performed on a regular message request from a user to another user
*/
void regular_handler(char buf[MAX_LINE], int len)
{
    /*
     TODO: should somehow send the message from src user to dst user
     src user should be authenticated
    */

   printf("server: regular message handler");
}

void handle_connection(int client_socket)
{
    char buf[MAX_LINE];
    bzero(buf, MAX_LINE);

    int len = recv(client_socket, buf, sizeof(buf), 0);
    
    // create a buffer to reply to client.
    char reply_buf[MAX_LINE] = "Reply from server\n";

    send(client_socket, reply_buf, MAX_LINE, 0); // send back to client

    fputs(buf, stdout); // output received message from client to stdout
}

void server_loop()
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
    while (1) server_loop();

    close(sfd); // should never get here though
}