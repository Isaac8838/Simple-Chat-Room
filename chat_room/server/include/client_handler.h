#ifndef _CLIENT_HANDLER_H_
#define _CLIENT_HANDLER_H_

#include "server.h"
#include "socket_conn.h"

/*
 * type of staus
 */

#define USER    0
#define GROUP   1
#define MAIL    2

/*
 * type of method
 */

#define SERVER_MESSAGE  0
#define CLIENT_MESSAGE  1
#define LOGIN_OR_SIGNUP 3

/*
 * type of command
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

/* 
 * client handler for handle client's commands or messages 
 */

void* clientHandler(void* arg);


/* 
 * login or sign up handlers 
 */

int login(struct User *user);
int signUp(struct User *user);

/*
 * command handler
 */
void commandHandler(struct User *user, struct Command *cmd);

/*
 * message handler
 */
void messageHandler(struct User *user, int group_id, char *message);

#endif
