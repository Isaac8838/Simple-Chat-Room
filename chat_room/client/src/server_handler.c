#include "../include/server_handler.h"

/*
 * Handling user request and response from server.
 * return -1 for error, otherwise 0.
 */
int server_handler(int sockfd) {
    struct Request  req;
    struct Response res;
    
    while (1) {

        /*
         * Getting user request.
         */
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
        if (recv(sockfd, &res, sizeof(struct Response), 0) < 0) {
            fprintf(stderr, "Error: receiving response from server failed.\n");
            perror("recv");
            return -1;
        }



    }

    return 0;
}