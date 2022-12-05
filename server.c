#include "server.h"

/* global variables */
int sfd;                        // fd for socket
struct sockaddr_in s_addr;      // address struct
struct user *user_list;         // list of authenticated users. notes: temporary solution
struct message *message_list;  // list of messages in flight
// maybe a semaphore for user_list + message_list syncrhonizaiton?

/*
 Helper to add message to the in-memory list of in-flight messages

 New messages added to tail of list.
 Messages closer to front are older messages.
*/
void add_message(struct user *from, struct user *to, char *message)
{
    struct message *new_message = (struct message*) malloc(sizeof(struct message));

    strncpy(new_message->buf, message, MAX_LINE);
    new_message->from = from;
    new_message->to = to;

    if(message_list == NULL) {
        new_message->prev = new_message;
        new_message->next = NULL;
        message_list = new_message;
    } else {
        new_message->next = NULL;
        new_message->prev = message_list->prev;
        message_list->prev->next = new_message;
        message_list->prev = new_message;
    }
}

/*
 Helper function to determine if string is empty or not.
 Reads only MAX_LINE characters
*/
int is_empty(char *str)
{
    return str == NULL || strnlen(str, MAX_LINE) == 0;
}

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
        cur_user = cur_user->next;
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
 message_handler

 Handles logic to be performed on a message request from a user to another user

 Handles D58P /Message requests

 D58P Message request form:

    D58P /Message
    <user>
    <password>
    <target user>
    <message>
*/
void message_handler(int client_socket, struct D58P *req, int len)
{
    struct D58P res;

    // get data from request
    char *username = req->lines[1];
    char *password = req->lines[2];
    char *recipient = req->lines[3];
    char *message = req->lines[4];

    // bad request if any fields empty
    if(is_empty(username) || is_empty(password) || is_empty(recipient) || is_empty(message)) {
        create_response(&res, D58P_GET_MESSAGE_STRING_RES, D58P_BAD_REQUEST);
        send_D58P_response(client_socket, &res);
        return;
    }

    // assure user is authenticated
    if(!is_authenticated(username, password)) {
        create_response(&res, D58P_GET_MESSAGE_STRING_RES, D58P_UNAUTHORIZED);
        send_D58P_response(client_socket, &res);
        return;
    }

    // make sure from and to exist
    struct user *from = find_user(username);
    struct user *to = find_user(recipient);

    if(from == NULL || to == NULL) {
        create_response(&res, D58P_GET_MESSAGE_STRING_RES, D58P_NOT_FOUND);
        send_D58P_response(client_socket, &res);
        return;
    }

    // add message to in-memory list of in flight messages
    add_message(from, to, message);

    // reply with 200 OK
    create_response(&res, D58P_GET_MESSAGE_STRING_RES, D58P_OK);
    send_D58P_response(client_socket, &res);
}

/*
 get_message_handler

 Handles logic to be performed on a get message request

 Handles D58P /Get Message requests

 D58P Message request form:
    D58P /Message
    <user>
    <password>
*/
void get_message_handler(int client_socket, struct D58P *req, int len)
{
    struct D58P res;
    bzero(&res, sizeof(struct D58P));

    char *username = req->lines[1];
    char *password = req->lines[2];

    // bad request if any fields empty
    if(is_empty(username) || is_empty(password)) {
        create_response(&res, D58P_GET_MESSAGE_STRING_RES, D58P_BAD_REQUEST);
        send_D58P_response(client_socket, &res);
        return;
    }

    // assure user is authenticated
    if(!is_authenticated(username, password)) {
        create_response(&res, D58P_GET_MESSAGE_STRING_RES, D58P_UNAUTHORIZED);
        send_D58P_response(client_socket, &res);
        return;
    }

    // find oldest message that should be sent
    struct message *cur_message, *prev_message, *oldest_message;
    
    prev_message = NULL;
    cur_message = message_list; // search from tail of list
    oldest_message = NULL;

    struct user *user = find_user(username);

    while(oldest_message == NULL) { // while we havent found a message
        while(cur_message != NULL) {
            if(cur_message->to == user) {
                oldest_message = cur_message;
                break;
            }

            prev_message = cur_message;
            cur_message = cur_message->next;
        }
        
        // reset values to search again
        cur_message = message_list;
        prev_message = NULL;
    }

    // remove message from message list
    if(message_list->next == NULL && oldest_message == message_list) {
        message_list = NULL;
    } else if(oldest_message == message_list->prev) {
        oldest_message->prev->next = oldest_message->next;
        message_list->prev = oldest_message->prev;
    } else {
        oldest_message->prev->next = oldest_message->next;
    }

    // reply with message
    create_get_message_response(&res, D58P_OK, oldest_message->from->username, oldest_message->buf);

    send_D58P_response(client_socket, &res);

    // free oldest message
    free(oldest_message);
}

/*
 Handles a connection from a client
*/
void *handle_connection(void *arg)
{
    int client_socket = *((int*) arg);

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
        message_handler(client_socket, &req, len);
    } else if (strncmp(req.lines[0], D58P_USER_STRING_REQ, MAX_REQUEST) == 0) {
        user_handler(client_socket, &req, len);
    } else if (strncmp(req.lines[0], D58P_GET_MESSAGE_STRING_REQ, MAX_REQUEST) == 0) {
        get_message_handler(client_socket, &req, len);
    } else {
        printf("Invalid request\n");
        char reply_buf[MAX_REQUEST] = "Invalid request\n";
        send(client_socket, reply_buf, MAX_REQUEST, 0); // send back to client
    }

    close(client_socket);
}

/*
 server_loop

 Constantly accepts connections from clients to process D58P requests
*/
void server_loop()
{
    int client_socket; pthread_t tid;
    
    client_socket = accept_connection(sfd, &s_addr);

    pthread_create(&tid, NULL, &handle_connection, &client_socket);
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