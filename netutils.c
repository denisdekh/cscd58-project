#include "netutils.h"

/*
 create_connection

 Creates connection to socket address. Returns socket file desciptor for connection.
 Terminates program if could not connect.

 Note: close(sfd) should be called later to close connection.
*/
int create_connection(struct sockaddr_in *sin)
{
    int sfd;

    // create socket
    if ((sfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("ERROR: create_connection could not create socket");
        exit(EXIT_FAILURE);
    }

    // connect to server
    if (connect(sfd, (struct sockaddr *)sin, sizeof(*sin)) < 0) {
        perror("ERROR: create_connection could not connect to server");
        close(sfd);
        exit(EXIT_FAILURE);
    }

    return sfd;
}

/*
 create_accept_connectionconnection

 Accepts a connection on specified socket file descriptor (sfd).
 Sets sin to the address of the connecting peer. 
 Returns new socket file descriptor for the client connection.
 Terminates program if could not accept.

 Note: close(cfd) should be called later to close connection.
*/
int accept_connection(int sfd, struct sockaddr_in *sin)
{
    int cfd;

    // accept from sin on socket sfd
    if ((cfd = accept(sfd, (struct sockaddr *)sin, (socklen_t*)sin)) < 0) {
        perror("ERROR: accept_connection could not accept client");
        exit(EXIT_FAILURE);
    }

    return cfd;
}

/*
 Sends a D58P request given its struct to provided server.
 Stores response in res.
 Returns length of response.
*/
int send_D58P_request(struct sockaddr_in *sin, struct D58P *req, struct D58P *res)
{
    // convert struct D58P into char buffer
    char buf[MAX_REQUEST];
    bzero(buf, MAX_REQUEST);
    int len = 0;

    for(int i = 0; i<MAX_LINES; i++)
    {
        if(strlen(req->lines[i]) == 0) break;

        len += strlen(req->lines[i]) + 1;

        if(len >= MAX_REQUEST) {
            perror("ERROR: send_D58P_request could not send request of size greater than MAX_REQUEST");
            exit(EXIT_FAILURE);
        }

        strcat(buf, req->lines[i]);
        strcat(buf, "\n");
    }

    len -= 1; // dont send the very last \n character

    // create connection to server
    int sfd = create_connection(sin);

    // send request
    send(sfd, buf, len, 0);

    // response from server
    char res_buf[MAX_REQUEST];
    bzero(res_buf, MAX_REQUEST);
    int res_len = recv(sfd, res_buf, MAX_REQUEST, 0);

    // acknowledge to server that we received a response
    send_D58P_response_ack(sfd);

    close(sfd);

    parse_D58P_buf(res, res_buf);

    return res_len;
}

/*
 Sends a D58P resposne given its struct to client connected to provided socked file descriptor
*/
void send_D58P_response(int sfd, struct D58P *res)
{
    char buf[MAX_REQUEST];
    bzero(buf, MAX_REQUEST);
    int len = 0;

    for(int i = 0; i<MAX_LINES; i++)
    {
        if(strlen(res->lines[i]) == 0) break;

        len += strlen(res->lines[i]) + 1;

        if(len >= MAX_REQUEST) {
            perror("ERROR: send_D58P_response could not send response of size greater than MAX_REQUEST");
            exit(EXIT_FAILURE);
        }

        strcat(buf, res->lines[i]);
        strcat(buf, "\n");
    }

    len -= 1; // dont send the very last \n character

    // send request
    send(sfd, buf, len, 0);
}

/*
 Helper to send a D58P response acknowledgement
*/
void send_D58P_response_ack(int sfd)
{
    // send request
    send(sfd, D58P_ACK_STRING, sizeof(D58P_ACK_STRING), 0);
}

/*
 Verifies an acknowledgement on the socket file descriptor

 returns 1 if acknowledgement received, 0 otherwise
*/
int verify_acknowledgement(int sfd)
{
    // acknowledgement
    char ack_buf[MAX_REQUEST];
    bzero(ack_buf, MAX_REQUEST);

    // recieve data  through socket
    recv(sfd, ack_buf, sizeof(ack_buf), 0); 
    
    // parse data as D58P request
    struct D58P ack;
    parse_D58P_buf(&ack, ack_buf);

    return strncmp(ack.lines[0], D58P_ACK_STRING, MAX_REQUEST) == 0;
}

/*
 create_message_request

 Builds a D58P /Message request from provided auth information and message information
*/
void create_message_request(struct D58P *req, struct D58P_auth *auth, struct D58P_message_data *data)
{
    // zero the struct
    bzero(req, sizeof(struct D58P));

    // set request type
    strncpy(req->lines[0], D58P_MESSAGE_STRING_REQ, sizeof(D58P_MESSAGE_STRING_REQ));

    // set username
    strncpy(req->lines[1], auth->username, auth->user_len);

    // set password
    strncat(req->lines[2], auth->password, auth->password_len);

    // set target user
    strncat(req->lines[3], data->target_user, data->target_user_len);

    // set message
    strncat(req->lines[4], data->message, data->message_len);
}

/*
 create_user_request

 Builds a D58P /User request from provided auth information
*/
void create_user_request(struct D58P *req, struct D58P_auth *auth)
{
    // zero the struct
    bzero(req, sizeof(struct D58P));

    // set request type
    strncpy(req->lines[0], D58P_USER_STRING_REQ, sizeof(D58P_USER_STRING_REQ));

    // set username
    strncpy(req->lines[1], auth->username, auth->user_len);

    // set password
    strncat(req->lines[2], auth->password, auth->password_len);

    // set e (public key)
    strncat(req->lines[3], auth->e, MAX_REQUEST);

    // set n (public key)
    strncat(req->lines[4], auth->n, MAX_REQUEST);
}

/*
 create_get_messages_request

 Builds a D58P /Get Message request from provided auth information
*/
void create_get_messages_request(struct D58P *req, struct D58P_auth *auth)
{
    // zero the struct
    bzero(req, sizeof(struct D58P));

    // set request type
    strncpy(req->lines[0], D58P_GET_MESSAGE_STRING_REQ, sizeof(D58P_GET_MESSAGE_STRING_REQ));

    // set username
    strncpy(req->lines[1], auth->username, auth->user_len);

    // set password
    strncat(req->lines[2], auth->password, auth->password_len);
}

/*
 Creates generic D58P response of specified type with specified response code
*/
void create_response(struct D58P *res, char *type, enum D58P_ResponseCode code)
{
    // zero the struct
    bzero(res, sizeof(struct D58P));

    // set request type
    int type_len = strlen(type);
    strncpy(res->lines[0], type, type_len);

    // set response code
    char res_code[MAX_REQUEST];
    sprintf(res_code, "%d", code);
    int code_len = strnlen(res_code, MAX_LINE);
    strncpy(res->lines[1], res_code, code_len);
}

/*
 Creates get message response given resposne code and the message to be returned
*/
void create_get_message_response(struct D58P *res, enum D58P_ResponseCode code, char *from, char buf[MAX_LINE])
{
    create_response(res, D58P_GET_MESSAGE_STRING_RES, code);

    // set sender of message
    strncpy(res->lines[2], from, MAX_LINE);

    // set message
    strncpy(res->lines[3], buf, MAX_LINE);
}

/*
 Takes lines from buf into D58P struct */
void parse_D58P_buf(struct D58P *data, char buf[MAX_REQUEST])
{
    bzero(data, sizeof(struct D58P));

    const char delim[] = "\n";

    // make a copy of buf so strtok isn't modified
    char buf_cpy[MAX_REQUEST];
    strncpy(buf_cpy, buf, MAX_REQUEST);

    char *token = strtok(buf_cpy, delim);

    int i = 0;
    while(token != NULL && i < MAX_LINES) {
        strncpy(data->lines[i], token, MAX_LINE);
        i++;
        token = strtok(NULL, delim);
    }
}

/*
 Prints a D58P request/response to stdout
*/
void dump_D58P(struct D58P *data)
{
    for(int i = 0; i<MAX_LINES; i++)
    {
        if(strlen(data->lines[i]) == 0) break;

        printf("%s\n", data->lines[i]);
    }
}
