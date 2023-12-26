#include "../include/server_handler.h"

static void serverMessageHandler(int sockfd, struct Response *res);
static int logoutHandler(int sockfd);
static int listGroupHandler(int sockfd);
static int joinGroupHandler(int sockfd, int user_id);
static int listMailHandler(int sockfd);

/*
 * Handling user request and response from server.
 * return -1 for error, otherwise 0.
 */
int serverHandler(int sockfd) {
    struct Request  req;
    struct Response res;
    
    while (1) {

        /*
         * Getting user request.
         */
        memset(&req, 0, sizeof(struct Request));
        if (fgets(req.request, sizeof(req.request), stdin) == NULL) {
            fprintf(stderr, "Error: user input from server handler failed.\n");
            perror("fgets");
            return -1;
        }

        /*
         * Sending request to server.
         */
        if (send(sockfd, &req, sizeof(struct Request), 0) < 0) {
            fprintf(stderr, "Error: sending request to server failed.\n");
            perror("send");
            return -1;
        }

        /*
         * Receiving response from server.
         */
        memset(&res, 0, sizeof(struct Response));
        if (recv(sockfd, &res, sizeof(struct Response), 0) < 0) {
            fprintf(stderr, "Error: receiving response from server failed.\n");
            perror("recv");
            return -1;
        }

        if (res.method == SERVER_MESSAGE) {

            serverMessageHandler(sockfd, &res);

        } else if (res.method == LOGOUT) {

            if (logoutHandler(sockfd) < 0) {
                fprintf(stderr, "Error: couldn't logout, please try again.\n");
                return -1;
            } else {
                break;
            }

        } else if (res.method == LIST_GROUP) {

            if (listGroupHandler(sockfd) < 0) {
                fprintf(stderr, "Error: couldn't printing, please try again.\n");
                return -1;
            }

        } else if (res.method == MESSAGE) {
            continue;
        } else if (res.method == JOIN_GROUP) {

            if (joinGroupHandler(sockfd, res.user_id) < 0) {
                fprintf(stderr, "Error: joining group faild.\n");
                return -1;
            }

        } else if (res.method == ERROR) {
            printf("Error happened.\n");
            return -1;
        } else if (res.method == LIST_MAIL) {
            if (listMailHandler(sockfd) < 0) {
                fprintf(stderr, "Error: list mails faild.\n");
                return -1;
            }
        }

    }

    return 0;
}

/*
 * Handling message from server
 */
static void serverMessageHandler(int sockfd, struct Response *res) {
    printf("%s", res->server_message);
    fflush(stdout);
}

/*
 * Handling client logout action
 * return -1 for error, otherwise return 0.
 */
static int logoutHandler(int sockfd) {
    struct Response res;
    struct Request  req;

    /*
     * Receiving logout successfully message from server
     */
    if (recv(sockfd, &res, sizeof(struct Response), 0) < 0) {
        fprintf(stderr, "Error: receiving logout message from server failed.\n");
        perror("recv");
        return -1;
    }
    printf("%s", res.server_message);

    /*
     * Receiving close message from server
     * If message match "close" string
     * return 0 for end the client, otherwise -1.
     */
    memset(&req, 0, sizeof(struct Request));
    if (recv(sockfd, &req, sizeof(struct Request), 0) < 0) {
        fprintf(stderr, "Error: receiving logout message from server failed.\n");
        perror("recv");
        return -1;
    }
    if (strcmp(req.request, "close") == 0) {
        return 0;
    } else {
        fprintf(stderr, "Error: the response message from server doesn't match.\n");
        return -1;
    }

    return -1;

}

/*
 * Printing groups.
 * return -1 for error, otherwise return 0.
 */
static int listGroupHandler(int sockfd) {
    struct Response res;
    
    /*
     * Receiving start list message from server.
     */
    memset(&res, 0, sizeof(struct Response));
    if (recv(sockfd, &res, sizeof(struct Response), 0) < 0) {
        fprintf(stderr, "Error: receiving start lists message failed.\n");
        perror("recv");
        return -1;
    }

    if (strcmp(res.server_message, "start list") == 0) {

        printf("%-20s%-20s\n", "Group name", "Owner");
        fflush(stdout);

        while (1) {

            memset(&res, 0, sizeof(res));
            if (recv(sockfd, &res, sizeof(struct Response), 0) < 0) {
                fprintf(stderr, "Error: receiving group lists from server failed.\n");
                perror("recv");
                return -1;
            }

            if (strcmp(res.server_message, "end list") == 0) {
                break;
            }

            printf("%-20.*s%-20.*s\n", (int)sizeof(res.message.groups), res.message.groups, (int)sizeof(res.message.names), res.message.names);
            fflush(stdout);
        }

    } else {
        return -1;
    }
    
    return 0;

}

/*
 * Handling join a group
 * Handler will create a new thread dealing with messages.
 */
static int joinGroupHandler(int sockfd, int user_id) {
    pthread_t thread;

    int *arg = malloc(sizeof(int));
    *arg = user_id;

    if (pthread_create(&thread, NULL, groupMessageHandler, (void *)arg) != 0) {
        fprintf(stderr, "Error: create group message handler failed.\n");
        return -1;
    }

    return 0;
}

static int listMailHandler(int sockfd) {
    struct Response res;

    printf("%-20s%-20s\n", "sender", "message");
    fflush(stdout);
    while (1) {
        memset(&res, 0, sizeof(res));
        if (recv(sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: receiving list mail message failed.\n");
            return -1;
        }

        if (strcmp(res.server_message, "end") == 0) {
            break;
        }

        printf("%-20s%-20s\n", res.message.names, res.message.messages);
        fflush(stdout);
    }

    return 0;

}