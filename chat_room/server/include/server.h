#ifndef _SERVER_H_
#define _SERVER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <mysql/mysql.h>

#define PORT 8888
#define MAX_CLIENT 100

extern int              has_registered[MAX_CLIENT];
extern int              User_online;
extern pthread_mutex_t  mutex;

struct User;
struct Message;
struct Response;

/*
 * user structure
 */
struct User {
    char    name[255];
    int     id;
    int     status;
    int     sockfd;
    MYSQL   *db;
};

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


#endif
