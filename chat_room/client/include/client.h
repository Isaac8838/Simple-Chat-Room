#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8888

struct Message;
struct Response;
struct Request;

/*
 * message structure
 */
struct Message {
    char    **names;
    char    **messages;
    char    **groups;
    char    **users;
    char    **mails;
};

/*
 * response structure
 */
struct Response {
    char    server_message[BUFSIZ];
    char    client_message[BUFSIZ];
    struct  Message message[100];
    int     method;
    int     user_id;
};

/*
 * request structure
 */
struct Request {
    char request[BUFSIZ];
};

#endif
