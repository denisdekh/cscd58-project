#include "netutils.h"
#include <netdb.h>
#include <string.h>

#define SERVER_PORT 6777
#define MAX_LINE 256
#define EXIT_COMMAND "/exit\n"

/* global variables */
struct sockaddr_in server_addr;     // server address struct

/*
 client loop
 return 0: successful client loop
 return 1: terminate loop
*/
int client_loop()
{
    char buf[MAX_LINE];
    int len, exit_flag = 0;
    
    fgets(buf, sizeof(buf), stdin);

    buf[MAX_LINE-1] = '\0';
    len = strlen(buf) + 1;

    if(strcmp(buf, EXIT_COMMAND) == 0) { // exit client
        exit_flag = 1;
    }

    if(exit_flag) return 1;

    // connect to server
    int sfd = create_connection(&server_addr);

    send(sfd, buf, len, 0);

    len = recv(sfd, buf, sizeof(buf), 0);
    fputs(buf, stdout);

    close(sfd);

    return 0;
}



int main(int argc, char * argv[])
{
    struct hostent *hp;
    char *host;

    if(argc != 2) {
        fprintf(stderr, "usage: client host\n");
        exit(EXIT_FAILURE);
    }

    host = argv[1];

    // resolve host to IP
    hp = gethostbyname(host);
    if (!hp) {
        fprintf(stderr, "client: unknown host: %s\n", host);
        exit(EXIT_FAILURE);
    }
    
    // setup server struct
    bzero((char *)&server_addr, sizeof(server_addr)); // erase bytes in sin
    server_addr.sin_family = AF_INET; // internet family
    bcopy(hp->h_addr, (char *)&server_addr.sin_addr, hp->h_length); // copy from hp to sin
    server_addr.sin_port = htons(SERVER_PORT); // set server port
    
    // main client loop
    while (1) if (client_loop() == 1) break;

    exit(EXIT_SUCCESS);
}