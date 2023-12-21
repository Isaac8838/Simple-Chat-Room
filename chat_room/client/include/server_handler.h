#ifndef _SERVER_HANDLER_H_
#define _SERVER_HANDLER_H_

#include "client.h"

/*
 * Server method
 * accroding to method, use the function with that server.
 */

#define CREATE_GROUP 0
#define LIST_GROUP 1
#define JOIN_GROUP 2
#define LOGOUT 3
#define LEAVE_GROUP 4
#define LIST_USER 5
#define INVITE 6
#define DELETE_GROUP 7
#define LIST_MAIL 8
#define MAILTO 9
#define HELP 10
#define COMMAND_NOT_FOUND 11
#define SERVER_MESSAGE  12
#define CLIENT_MESSAGE  13
#define LOGIN_OR_SIGNUP 14

int serverHandler(int sockfd);

#endif
