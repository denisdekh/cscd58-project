#include "server.h"

/* global variables */
int sfd;                        // fd for socket
struct sockaddr_in s_addr;      // address struct


/*
 user_handler

 Handles logic to be performed on a user request. (login / register)

 Handles D58P /User requests

 D58P User request form:

    D58P /User
    <user>
    <password>
*/
void user_handler(char buf[MAX_REQUEST], int len)
{
    /*
     TODO: should allow requesting user to login/register

     If the <user> exists in the server and <password> is valid then login as this user.
     If the <user> does not exist in the server then register a new user with <user>/<password> and login
    */

    printf("server: user handler\n");
    const char delim[] = "\n";
    char *token = strtok(buf, delim);

    while(token != NULL) {
        printf("%s\n", token);
        token = strtok(NULL, delim);
    }
}

/*
 regular_handler

 Handles logic to be performed on a regular message request from a user to another user

 Handles D58P /Message requests

 D58P Message request form:

    D58P /Message
    <user>
    <password>
    <target user>
    <message>
*/
void regular_handler(char buf[MAX_REQUEST], int len)
{
    printf("server: regular message handler\n");
    const char delim[] = "\n";
    char *token = strtok(buf, delim);

    while(token != NULL) {
        printf("%s\n", token);
        token = strtok(NULL, delim);
    }
}

/*
 Handles a connection from a client
*/
void handle_connection(int client_socket)
{
    char buf[MAX_REQUEST];
    bzero(buf, MAX_REQUEST);

    int len = recv(client_socket, buf, sizeof(buf), 0);
    
    // create a buffer to reply to client.
    const char delim[] = "\n";

    char buf_cpy[MAX_REQUEST];
    strncpy(buf_cpy, buf, MAX_REQUEST);

    char *token = strtok(buf_cpy, delim);

    if(strncmp(token, D58P_MESSAGE_STRING, MAX_REQUEST) == 0) {
        regular_handler(buf, len);
    } else if (strncmp(token, D58P_USER_STRING, MAX_REQUEST) == 0) {
        user_handler(buf, len);
    } else {
        char reply_buf[MAX_REQUEST] = "Invalid request\n";
        send(client_socket, reply_buf, MAX_REQUEST, 0); // send back to client
    }
}

/*
 server_loop

 Constantly accepts connections from clients to process D58P requests
*/
void server_loop()
{
    int client_socket = accept_connection(sfd, &s_addr);
    printf("~ client request ~\n");
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