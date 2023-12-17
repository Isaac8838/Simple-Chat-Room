#include "../include/client.h"
#include "../include/socket_conn.h"

int main() {
    int client_socket;
    struct sockaddr_in server;

    client_socket = sockConnection(&server);

    while (1) {



    }

    close(client_socket);
    return 0;
}