#include "../include/server_handler.h"

static void serverMessageHandler(int sockfd, struct Response *res);
static int logoutHandler(int sockfd, struct Response *res);
static int listGroupHandler(int sockfd, struct Response *res);

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

            if (logoutHandler(sockfd, &res) < 0) {
                fprintf(stderr, "Error: couldn't logout, please try again.\n");
                return -1;
            } else {
                break;
            }

        } else if (res.method == LIST_GROUP) {

            if (listGroupHandler(sockfd, &res) < 0) {

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
}

/*
 * Handling client logout action
 * return -1 for error, otherwise return 0.
 */
static int logoutHandler(int sockfd, struct Response *res) {

    printf("%s", res->server_message);

    if (recv(sockfd, &(*res), sizeof(struct Response), 0) < 0) {
        fprintf(stderr, "Error: receiving logout message from server failed.\n");
        perror("recv");
        return -1;
    }

    if (strcmp(res->server_message, "close") == 0) {
        return 0;
    } else {
        fprintf(stderr, "Error: the response message from server doesn't match.\n");
        return -1;
    }

    return -1;

}

/*
 * printing groups.
 */
static int listGroupHandler(int sockfd, struct Response *res) {

    if (strcmp(res->server_message, "start list") == 0) {

        printf("group name\t\towner\n");

        while (1) {

            if (recv(sockfd, &(*res), sizeof(struct Response), 0) < 0) {
                fprintf(stderr, "Error: receiving group lists from server failed.\n");
                perror("recv");
                return -1;
            }

            if (strcmp(res->server_message, "end list") == 0) {
                break;
            }

            printf("%s\t\t%s\n", res->message.groups, res->message.names);

        }

    } else {
        return -1;
    }
    
    return 0;

}