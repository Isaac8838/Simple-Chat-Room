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
    char    names[255];
    char    messages[BUFSIZ];
    char    groups[255];
    char    users[255];
    char    mails[BUFSIZ];
};

/*
 * response structure
 */
struct Response {
    char    server_message[BUFSIZ + BUFSIZ];
    char    client_message[BUFSIZ + BUFSIZ];
    struct  Message message;
    int     method;
    int     user_id;
    int     number_of_messages;
};

/*
 * request structure
 */
struct Request {
    char request[BUFSIZ];
};

#endif
