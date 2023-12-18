#include "../include/client.h"
#include "../include/socket_conn.h"

static int LoginOrSignUpHandler(int sockfd);

int main() {
    int client_socket;
    struct sockaddr_in server;

    client_socket = sockConnection(&server);

    while (1) {



    }

    close(client_socket);
    return 0;
}

static int LoginOrSignUpHandler(int sockfd) {
    struct Response response;

    while (1) {

    }
}