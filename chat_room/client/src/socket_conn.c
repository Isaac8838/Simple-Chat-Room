#include "../include/socket_conn.h"

/*
 * Create socket connection
 * reutrn socket file descriptor
 */
int sockConnection(struct sockaddr_in *server) {
    int sockfd;

    memset(&(*server), 0, sizeof(struct sockaddr_in));
    server->sin_family = PF_INET;
    server->sin_port = htons(PORT);
    inet_aton("127.0.0.1", &(server->sin_addr));

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error: socket creation failed.\n");
        perror("socket");
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&(*server), sizeof(*server))) {
        fprintf(stderr, "Error: connecting to server failed.\n");
        perror("connect");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}