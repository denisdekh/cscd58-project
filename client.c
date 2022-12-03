#include "client.h"

/* global variables */
struct sockaddr_in server_addr;     // server address struct

void user_handler(char buf[MAX_LINE], int len)
{
    /*
     TODO: should communicate with server to somehow identify the client with the server
     Messages of form:
        /user <user> <password>
    */

    printf("user handler\n");
}

void msg_handler(char buf[MAX_LINE], int len)
{
    /*
     TODO: should set a global variable within client so that future messages to regular_handler will send messages
     to the specified user. (no server communications required)

     Messages of form:
        /msg <user>
     
     All further messages should be directed to specified user
    */
    
    printf("msg handler\n");
}

void regular_handler(char buf[MAX_LINE], int len)
{
    /*
     TODO: should communicate with server to somehow send a message to the user the client is chatting with 
     
     client should be logged in with /user
     client should be chatting with someone using /msg <user>
    */

    printf("regular handler\n");

    // connect to server
    int sfd = create_connection(&server_addr);

    send(sfd, buf, len, 0);

    len = recv(sfd, buf, MAX_LINE, 0);

    fputs(buf, stdout);

    close(sfd);
}

/*
 exit_handler
 Exits the client when user inputs /exit 
*/
void exit_handler(char buf[MAX_LINE], int len)
{
    exit(EXIT_SUCCESS);
}

/*
 parse_input
 returns the type of message inputed by the user
*/
int parse_input(char buf[MAX_LINE])
{
    if (strncmp(buf, EXIT_COMMAND, sizeof(EXIT_COMMAND)) == 0) {
        return INPUT_EXIT;
    } else if (strncmp(buf, USER_COMMAND, sizeof(USER_COMMAND) - 1) == 0) {
        return INPUT_USER;
    } else if (strncmp(buf, MESSAGE_COMMAND, sizeof(MESSAGE_COMMAND) - 1) == 0) {
        return INPUT_MSG;
    }

    return INPUT_REGULAR;
}

/*
 client_loop
 return 0: successful client loop
 return 1: terminate loop
*/
void client_loop()
{
    char buf[MAX_LINE];
    int len, exit_flag = 0;
    
    fgets(buf, sizeof(buf), stdin);

    buf[MAX_LINE-1] = '\0';
    len = strlen(buf);

    switch (parse_input(buf))
    {
        case INPUT_USER:
            return user_handler(buf, len);
        case INPUT_EXIT:
            return exit_handler(buf, len);
        case INPUT_MSG:
            return msg_handler(buf, len);
        default:
            return regular_handler(buf, len);
    }
}

int main(int argc, char * argv[])
{
    struct hostent *hp;
    char *host;

    if (argc != 2) {
        fprintf(stderr, "usage: client <host>\n");
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
    bcopy(hp->h_addr_list[0], (char *)&server_addr.sin_addr, hp->h_length); // copy from hp to sin
    server_addr.sin_port = htons(SERVER_PORT); // set server port
    
    // main client loop
    while (1) client_loop();
}