#include "../include/socket_conn.h"

/*
 * Initialize server socket
 */
int socketInit(struct sockaddr_in *server) {
    int sockfd;

    memset(&(*server), 0, sizeof(struct sockaddr_in));
    server->sin_family = PF_INET;
    server->sin_port = htons(PORT);
    inet_aton("127.0.0.1", &(server->sin_addr));

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error: creating server socket failed.\n");
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (bind(sockfd, (struct sockaddr *)&(*server), sizeof(*server)) < 0) {
        fprintf(stderr, "Error: binding server address and port failed.\n");
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(sockfd, 100) < 0) {
        fprintf(stderr, "Error: listening server failed.\n");
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return sockfd;
}


/*
 * Find the smallest unused id.
 */
int unusedID() {
    int i;

    for (i = 0; i < MAX_CLIENT; i++) {
        if (has_registered[i] == 0) {
            break;
        }
    }

    return i;
}