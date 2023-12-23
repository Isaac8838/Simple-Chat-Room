#include "../include/server_handler.h"

static void serverMessageHandler(int sockfd, struct Response *res);
static int logoutHandler(int sockfd);
static int listGroupHandler(int sockfd);

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
    
    memset(&res, 0, sizeof(struct Response));
    if (recv(sockfd, &res, sizeof(struct Response), 0) < 0) {
        fprintf(stderr, "Error: receiving start lists message failed.\n");
        perror("recv");
        return -1;
    }

    if (strcmp(res.server_message, "start list") == 0) {

        printf("group name\t\towner\n");
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

            printf("%s\t\t%s\n", res.message.groups, res.message.names);
            fflush(stdout);
        }

    } else {
        return -1;
    }
    
    return 0;

}