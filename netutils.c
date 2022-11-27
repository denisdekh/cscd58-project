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