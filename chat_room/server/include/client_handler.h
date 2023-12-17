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

/* client handler for handle client's commands or messages */

void* clientHandler(void* arg);


/* login or sign up handlers */

int login(struct User *user);
int signUp(struct User *user);


#endif
