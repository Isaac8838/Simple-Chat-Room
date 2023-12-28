#include "../include/server_handler.h"

static int sockConn(struct sockaddr_in *server, int user_id);

/*
 * Handler will connect to server socket and receiving the message from server.
 */
void *groupMessageHandler(void* arg) {
    pthread_detach(pthread_self());
    int                 user_id = *((int *)arg);
    int                 sockfd;
    struct sockaddr_in  server;
    struct Response     res;
    free(arg);

    /*
     * Connecting to server
     */
    if ((sockfd = sockConn(&server, user_id)) < 0) {
        fprintf(stderr, "Error: connecting to group message server failed.\n");
        pthread_exit(NULL);
    }

    /*
     * Receiving message from server and output.
     * If server send "close" then terminate this handler.
     */
    while (1) {
        memset(&res, 0, sizeof(res));
        if (recv(sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: receiving group message failed.\n");
            break;
        }

        if (strcmp(res.server_message, "close") == 0) {
            break;
        }

        printf("%s: %s\n", res.message.names, res.message.messages);
        fflush(stdout);

    }

    close(sockfd);
    pthread_exit(NULL);
}

/*
 * Setting socket connection
 */
static int sockConn(struct sockaddr_in *server, int user_id) {
    int sockfd;

    memset(&(*server), 0, sizeof(struct sockaddr_in));
    server->sin_family = PF_INET;
    server->sin_port = htons(PORT + user_id);
    inet_aton("127.0.0.1", &(server->sin_addr));

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error: socket creation failed.\n");
        perror("socket");
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&(*server), sizeof(*server))) {
        fprintf(stderr, "Error: connecting to server failed.\n");
        perror("connect");
        return -1;
    }

    return sockfd;
}