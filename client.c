#include "client.h"

/* global variables */
struct sockaddr_in server_addr;     // server address struct
char target_user[MAX_LINE];
char target_key_name[MAX_LINE];
struct D58P_auth auth;
int authenticated;              // client side flag to determine if authenticated
pthread_t get_msg_tid;
RSA *keys, *target_key;


/*
 Routine for other thread.
 Uses long polling type strategy to get messages from server
*/
void* get_messages(void *aux)
{
    while(1) {
        if(authenticated) {
            // build get message request
            struct D58P req, res;
            create_get_messages_request(&req, &auth);

            int res_len = send_D58P_request(&server_addr, &req, &res);
            int code = atoi(res.lines[1]);
            char *from = res.lines[2];
            char *message = res.lines[3];
            
            //convert from hex
            long buflen = (long) strlen(message); 
            unsigned char *bin = OPENSSL_hexstr2buf(message, &buflen);

            unsigned char plaintext[MAX_LINE];
            int plain_len = decrypt_message(keys, bin, plaintext, 256);
            plaintext[plain_len] = '\0';

            // print the message
            if(code == D58P_OK) {
                printf("%s: %s\n", from, plaintext);
            }
        }
    }
}


 /*
     Should communicate with server to somehow identify the client with the server
     Messages of form:
        /user <user> <password>
    */
void user_handler(char buf[MAX_LINE], int len)
{
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
    
    if (get_public(keys, auth.e, auth.n)) {     // get the key and set it
        fprintf(stderr, "client: could not retrieve the public key\n");
        exit(EXIT_FAILURE);
    }

    // dont set \n in username and password
    auth.user_len += 1;
    auth.username[auth.user_len-1] = '\0';  // add a \0 to username
    auth.password[auth.password_len-1] = '\0'; // replace \n with \0

    // build request   
    struct D58P req, res;
    create_user_request(&req, &auth);

    int res_len = send_D58P_request(&server_addr, &req, &res);

    int code = atoi(res.lines[1]);
    authenticated = code == D58P_OK || code == D58P_CREATED;

    if(code == D58P_OK || code == D58P_CREATED) {
        printf("Authenticated as %s\n", auth.username);

        // cancel old thread if exists
        if(get_msg_tid) {
            pthread_cancel(get_msg_tid);
        }

        // create get messages thread
        pthread_create(&get_msg_tid, NULL, &get_messages, NULL);
    } else {
        printf("Authenticated failed\n");
        bzero(&auth, sizeof(struct D58P_auth));
    }
}


/*  Send request to the server of the form
    D58P /Get Key
    <user>
    <target_user>
    
    Then saves the key and the name of the target_user to keep track of the current recipient key*/
int key_handler() {
    if(authenticated) {
        // build get key request
        struct D58P req, res;
        create_get_key_request(&req, &auth, target_user);

        int res_len = send_D58P_request(&server_addr, &req, &res);
        int code = atoi(res.lines[1]);
        
        if(code == D58P_OK) {
            char *key_for = res.lines[3];
            char *e = res.lines[4];
            char *n = res.lines[5];
            RSA_free(target_key);
            target_key = RSA_new();
            if (set_public(target_key, e, n)) {  return 1; }
            strncpy(target_key_name, key_for, MAX_LINE);
            return 0;
        } else {
            return 1;
        }
    }
}

/*
    Sets a global variable target_user[MAX_LINE] within client so that future messages to send_message_handler
    will send messages to the specified user.

    Messages of form:
    /msg <recipient>
    
    All further messages should be directed to specified user*/
void msg_handler(char buf[MAX_LINE], int len)
{
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
        target_user[target_user_len] = '\0'; // in case the new target's name is shorter than previous

        if(target_key_name == NULL || strncmp(target_key_name, target_user, MAX_LINE)) {
            if (key_handler()) { 
                printf("Could not retrieve the key of user '%s'\n", user);
                return;
            }
        }

        
        
        printf("Now chatting with %s", user);
    }
}

/*
    should communicate with server to send message to user

    client should be logged in with /user
    client should be chatting with someone using /msg <user>
*/
void send_message_handler(char buf[MAX_LINE], int len)
{
    // build message data
    struct D58P_message_data data;

    // not authenticated
    if(authenticated && auth.user_len == 0 || auth.password_len == 0) {
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
    len -= 1;
    buf[len] = '\0'; // dont copy the \n at the end

    // encrypt the message
    unsigned char ciphertext[MAX_LINE];
    int cipher_len;
    encrypt_message(target_key, buf, ciphertext, len, &cipher_len);
    
    // convert to hex
    char *hex = OPENSSL_buf2hexstr(ciphertext, (long)cipher_len);
    int hex_len = strlen(hex);
    memcpy(data.message, hex, hex_len);
    data.message_len = hex_len;

    // build message request from message data
    struct D58P req, res;
    create_message_request(&req, &auth, &data);

    int res_len = send_D58P_request(&server_addr, &req, &res);

    int code = atoi(res.lines[1]);

    if(code != D58P_OK) {
        printf("Could not send message to %s\n", target_user);
    }
}

/*
 exit_handler
 Exits the client when user inputs /exit 
*/
void exit_handler(char buf[MAX_LINE], int len)
{
    RSA_free(keys);
    RSA_free(target_key);
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

    return INPUT_SEND_MESSAGE;
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
            return send_message_handler(buf, len);
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
    if (get_keys(&keys) || keys == NULL) {
        fprintf(stderr, "client: could not generate encryption keys\n");
        exit(EXIT_FAILURE);
    }

    printf("Chat client started...\n");
    printf("Please authenticate using /user <user> <password>\n");
    printf("Then begin chatting with a user using /msg <recipient>\n");
    printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

    // main client loop
    while (1) client_loop();
}
