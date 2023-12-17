#include "../include/server.h"
#include "../include/socket_conn.h"
#include "../include/client_handler.h"

int             has_registered[MAX_CLIENT];
int             User_online;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main() {
    int                 server_socket, client_socket;
    struct sockaddr_in  server, client;
    socklen_t           sin_siz;
    pthread_t           client_threads[MAX_CLIENT];

    server_socket = socketInit(&server);
    memset(has_registered, 0, sizeof(has_registered));
    User_online = 0;

    printf("Starting server...\n");

    while (1) {

        if (User_online == MAX_CLIENT) {
            continue;
        }

        /*
         * Accepting connection from clients.
         */
        if ((client_socket = accept(server_socket, (struct sockaddr *)&client, &sin_siz)) < 0) {
            fprintf(stderr, "Error: accepting client connection failed.\n");
            perror("accpet");
            exit(EXIT_FAILURE);
        }

        printf("connecting from: %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        int id = unusedID();

        struct User *user = malloc(sizeof(struct User));
        *user = (struct User){"None", id, -1, client_socket, NULL};

        /*
         * Creating a new thread to serve client
         */
        pthread_mutex_lock(&mutex);
        User_online++;
        pthread_mutex_unlock(&mutex);
        int error_code = pthread_create(&client_threads[id], NULL, clientHandler, user);
        if (error_code != 0) {
            fprintf(stderr, "Error: creating a client thread: %s.\n", strerror(error_code));
            free(user);
            pthread_mutex_lock(&mutex);
            User_online--;
            pthread_mutex_unlock(&mutex);
            break;
        }
    }

    close(server_socket);
    close(client_socket);
    return -1;
}
