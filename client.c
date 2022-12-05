#include "client.h"

/* global variables */
struct sockaddr_in server_addr;     // server address struct
char target_user[MAX_LINE];
struct D58P_auth auth;
RSA *keys;

void user_handler(char buf[MAX_LINE], int len)
{
    /*
     Should communicate with server to somehow identify the client with the server
     Messages of form:
        /user <user> <password>
    */

    const char delim[] = " ";
    strtok(buf, delim); // command
    char *user = strtok(NULL, delim); // arg1

    if (user == NULL) {
        printf("Usage: /user <user> <password>\n");
        return;
    }

    char *password = strtok(NULL, delim); // arg2

    if (password == NULL || strnlen(password, MAX_LINE) == 1) {
        printf("Usage: /user <user> <password>\n");
        return;
    }

    if (strtok(NULL, delim) != NULL) {
        printf("Usage: /user <user> <password>\n");
        return;
    }

    auth.user_len = strlen(user);
    auth.password_len = strlen(password);

    strcpy(auth.username, user);
    strcpy(auth.password, password);

    // dont set \n in username and password
    auth.user_len += 1;
    auth.username[auth.user_len-1] = '\0';  // add a \0 to username
    auth.password[auth.password_len-1] = '\0'; // replace \n with \0

    // build request   
    struct D58P req, res;
    create_user_request(&req, &auth);

    int res_len = send_D58P_request(&server_addr, &req, &res);
    printf("received response: \n");
    dump_D58P(&res);
}

void msg_handler(char buf[MAX_LINE], int len)
{
    /*
     Sets a global variable target_user[MAX_LINE] within client so that future messages to regular_handler will send messages
     to the specified user.

     Messages of form:
        /msg <recipient>
     
     All further messages should be directed to specified user
    */
    const char delim[] = " ";
    strtok(buf, delim); // command
    char *user = strtok(NULL, delim); // arg1

    if (strtok(NULL, delim) != NULL) {
        printf("Usage: /msg <recipient>\n");
    } else {
        if(user == NULL) {
            printf("Usage: /msg <recipient>\n");
            return;
        }

        int target_user_len = strnlen(user, MAX_LINE) - 1; // dont copy the \n at the end

        strncpy(target_user, user, target_user_len);
        printf("Now chatting with %s", user);
    }
}

void regular_handler(char buf[MAX_LINE], int len)
{
    /*
     should communicate with server to send message to user

     client should be logged in with /user
     client should be chatting with someone using /msg <user>
    */

    // build message data
    struct D58P_message_data data;

    // not authenticated
    if(auth.user_len == 0 || auth.password_len == 0) {
        printf("Authenticate using /user <user> <password>\n");
        return;
    }

    // set target user for message
    data.target_user_len = strnlen(target_user, MAX_LINE);
    if (data.target_user_len == 0) {
        printf("Initiate chat using /msg <recipient>\n");
        return;
    }

    if (len <= 1) return;

    strncpy(data.target_user, target_user, data.target_user_len);

    // set message content
    memcpy(data.message, buf, len);
    data.message_len = len - 1; // dont copy the \n at the end

    // build message request from message data
    struct D58P req, res;
    create_message_request(&req, &auth, &data);

    int res_len = send_D58P_request(&server_addr, &req, &res);
    
    // TODO: do something on response
    printf("sent message \n");
}

/*
 exit_handler
 Exits the client when user inputs /exit 
*/
void exit_handler(char buf[MAX_LINE], int len)
{
    RSA_free(keys);
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

    // zero global variables initially
    bzero((char *) &auth, sizeof(auth));
    bzero(target_user, sizeof(target_user));
    // get encryption keys
    if (get_keys(&keys)) {
        fprintf(stderr, "client: could not generate encryption keys\n");
        exit(EXIT_FAILURE);
    }
    /* Encryption test
    fprintf(stderr, "client: got keys %d\n", RSA_size(keys));

    char msg[] = "this is my message";
    unsigned char ciphertext[KEY_LENGTH/8];
    int cipher_len;
    encrypt_message(keys, msg, ciphertext, strlen(msg), &cipher_len);
    fprintf(stderr, "client: the original message = '%s'\nthe ciphertext = '%s'\n", msg, ciphertext);

    unsigned char plaintext[KEY_LENGTH/8];
    decrypt_message(keys, ciphertext, plaintext, cipher_len);
    fprintf(stderr, "client: the decrypted message = '%s'\n", plaintext); */


    printf("Chat client started...\n");
    printf("Please authenticate using /user <user> <password>\n");
    printf("Then begin chatting with a user using /msg <recipient> <message>\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    // main client loop
    while (1) client_loop();
}