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
        perror("client could not create socket");
        exit(EXIT_FAILURE);
    }

    // connect to server
    if (connect(sfd, (struct sockaddr *)sin, sizeof(*sin)) < 0) {
        perror("client could not connect to server");
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
        perror("server: could not accept client");
        exit(EXIT_FAILURE);
    }

    return cfd;
}


void create_message_request(struct D58P_request *req, struct D58P_auth *auth, struct D58P_message_data *data)
{
    char err_msg[] = "client: create_message_request, length of data is greater than MAX_REQUEST";

    // zero the struct
    bzero(req, MAX_REQUEST);

    int offset = 0;

    // set request type
    assert(offset + sizeof(D58P_MESSAGE_STRING) < MAX_REQUEST, err_msg);
    strncat(req->buf, D58P_MESSAGE_STRING, sizeof(D58P_MESSAGE_STRING));
    offset += sizeof(D58P_MESSAGE_STRING) - 1; // dont copy end of string '\0'

    assert(offset + sizeof("\n") < MAX_REQUEST, err_msg);
    strncat(req->buf + offset, "\n", sizeof("\n"));
    offset += sizeof("\n") - 1;

    // set username
    assert(offset + auth->user_len < MAX_REQUEST, err_msg);
    strncat(req->buf + offset, auth->username, auth->user_len);
    offset += auth->user_len - 1;

    assert(offset + sizeof("\n") < MAX_REQUEST, err_msg);
    strncat(req->buf + offset, "\n", sizeof("\n"));
    offset += sizeof("\n") - 1;

    // set password
    assert(offset + auth->password_len < MAX_REQUEST, err_msg);
    strncat(req->buf + offset, auth->password, auth->password_len);
    offset += auth->password_len - 1;

    assert(offset + sizeof("\n") < MAX_REQUEST, err_msg);
    strncat(req->buf + offset, "\n", sizeof("\n"));
    offset += sizeof("\n") - 1;

    // set target user
    assert(offset + data->target_user_len < MAX_REQUEST, err_msg);
    strncat(req->buf + offset, data->target_user, data->target_user_len);
    offset += data->target_user_len - 1;
   
    assert(offset + sizeof("\n") < MAX_REQUEST, err_msg);
    strncat(req->buf + offset, "\n", sizeof("\n"));
    offset += sizeof("\n") - 1;

    // set message
    assert(offset + data->message_len < MAX_REQUEST, err_msg);
    strncat(req->buf + offset, data->message, data->message_len);
    offset += data->message_len; // copy end of string \0 this time
    
    req->len = offset;
}


void create_user_request(struct D58P_request *req, struct D58P_auth *auth)
{
    char err_msg[] = "client: create_user_request, length of created request is greater than MAX_REQUEST";

    // zero the struct
    bzero(req, MAX_REQUEST);

    int offset = 0;

    // set request type
    assert(offset + sizeof(D58P_USER_STRING) < MAX_REQUEST, err_msg);
    strncat(req->buf, D58P_USER_STRING, sizeof(D58P_USER_STRING));
    offset += sizeof(D58P_USER_STRING) - 1; // dont copy end of string '\0'

    assert(offset + sizeof("\n") < MAX_REQUEST, err_msg);
    strncat(req->buf + offset, "\n", sizeof("\n"));
    offset += sizeof("\n") - 1;

    // set username
    assert(offset + auth->user_len < MAX_REQUEST, err_msg);
    strncat(req->buf + offset, auth->username, auth->user_len);
    offset += auth->user_len - 1;

    assert(offset + sizeof("\n") < MAX_REQUEST, err_msg);
    strncat(req->buf + offset, "\n", sizeof("\n"));
    offset += sizeof("\n") - 1;

    // set password
    assert(offset + auth->password_len < MAX_REQUEST, err_msg);
    strncat(req->buf + offset, auth->password, auth->password_len);
    offset += auth->password_len - 1;
    
    req->len = offset;
}
