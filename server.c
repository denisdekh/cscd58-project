#include "server.h"

/* global variables */
int sfd;                        // fd for socket
struct sockaddr_in s_addr;      // address struct
struct user *user_list;         // list of authenticated users. notes: temporary solution


/*
 Adds a new user to the linked list of users
*/
void register_user(char *username, char *password)
{
    struct user *new_user = (struct user*)malloc(sizeof(struct user));

    strncpy(new_user->username, username, MAX_LINE);
    strncpy(new_user->password, password, MAX_LINE);
    
    new_user->next = user_list;
    user_list = new_user;
}

/*
 Returns user struct for specified username if it exists
*/
struct user* find_user(char *username)
{
    struct user *cur_user = user_list;

    while(cur_user != NULL) {
        if(strncmp(username, cur_user->username, MAX_LINE) == 0) {
            return cur_user;
        }
    }

    return NULL;
}

/*
 Temporary solution to determining if a user is authenticated or not

 Uses global linked list to determine if a user exists or not
*/
int is_authenticated(char *username, char *password)
{
    struct user *user = find_user(username);

    if(user == NULL) {
        return 0;
    }

    return strncmp(password, user->password, MAX_LINE) == 0;
}

/*
 user_handler

 Handles logic to be performed on a user request. (login / register)
 Currently uses a temporary solution with a global linked list for storing users.

 Handles D58P /User requests

 D58P User request form:

    D58P /User
    <user>
    <password>
*/
void user_handler(int client_socket, struct D58P *req, int len)
{
    struct D58P res;
    char *username = req->lines[1];
    char *password = req->lines[2];

    // handle invalid username / password
    if(username == NULL || strnlen(username, MAX_LINE) == 0) { // invalid username
        create_response(&res, D58P_USER_STRING_RES, D58P_BAD_REQUEST);
        send_D58P_response(client_socket, &res);
        return;
    }

    if(password == NULL || strnlen(password, MAX_LINE) == 0) { // invalid password
        create_response(&res, D58P_USER_STRING_RES, D58P_BAD_REQUEST);
        send_D58P_response(client_socket, &res);
        return;
    }
    
    // perform action
    if(is_authenticated(username, password)) { // logged in
        create_response(&res, D58P_USER_STRING_RES, D58P_OK);
    } else if(find_user(username) == NULL) { // register new user
        register_user(username, password);
        create_response(&res, D58P_USER_STRING_RES, D58P_CREATED);
    } else {  // unauthenticated
        create_response(&res, D58P_USER_STRING_RES, D58P_UNAUTHORIZED);
    }

    send_D58P_response(client_socket, &res);
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
void regular_handler(int client_socket, struct D58P *req, int len)
{
    printf("server: regular message handler\n");
    // TODO: send a message somehow
}

/*
 Handles a connection from a client
*/
void handle_connection(int client_socket)
{
    char buf[MAX_REQUEST];
    bzero(buf, MAX_REQUEST);

    // recieve data  through socket
    int len = recv(client_socket, buf, sizeof(buf), 0); 
    
    // parse data as D58P request
    struct D58P req;
    parse_D58P_buf(&req, buf);
    
    printf("~ Received request ~\n");
    dump_D58P(&req);

    // Go to request handler
    if(strncmp(req.lines[0], D58P_MESSAGE_STRING_REQ, MAX_REQUEST) == 0) {
        regular_handler(client_socket, &req, len);
    } else if (strncmp(req.lines[0], D58P_USER_STRING_REQ, MAX_REQUEST) == 0) {
        user_handler(client_socket, &req, len);
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