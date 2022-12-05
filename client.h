#include "common.h"
#include "encrypt.h"
#include <netdb.h>

#ifndef CLIENT_H
#define CLIENT_H

/*
 Command syntax is:

 /user <user> <password>
 /exit
 /msg <user>

 Then any other message would be sent to whoever the user is chatting with

*/

#define USER_COMMAND "/user"
#define EXIT_COMMAND "/exit\n"
#define MESSAGE_COMMAND "/msg"

enum InputType {
    INPUT_SEND_MESSAGE,
    INPUT_USER,
    INPUT_EXIT,
    INPUT_MSG
};

#endif