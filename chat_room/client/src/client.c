#include "../include/client.h"
#include "../include/server_handler.h"
#include "../include/socket_conn.h"

static int LoginOrSignUpHandler(int sockfd);

int main() {
    int     client_socket;
    struct  sockaddr_in server;

    client_socket = sockConnection(&server);
    
    if (LoginOrSignUpHandler(client_socket) < 0) {
        fprintf(stderr, "Error: login or sign up failed.\n");
        exit(EXIT_FAILURE);
    }

    if (serverHandler(client_socket) < 0) {
        fprintf(stderr, "Error: server handler failed.\n");
        exit(EXIT_FAILURE);
    }

    close(client_socket);
    return 0;
}

/*
 * Handling the Login or Sign up
 * return -1 for error, otherwise 0.
 */
static int LoginOrSignUpHandler(int sockfd) {
    struct      Response response;
    char        server_message[BUFSIZ];
    char        client_message[BUFSIZ];
    const char  login_success[BUFSIZ] = "Login successfully!\n";
    const char  sign_up_success[BUFSIZ] = "Sign up successfully!\n";
    const char  login_failed[BUFSIZ] = "User doesn't exsit.\n";
    const char  sign_up_failed[BUFSIZ] = "User has been registered.\n";

    while (1) {

        /*
         * receiving server response for Login or Sign up.
         */
        memset(&response, 0, sizeof(struct Response));
        if (recv(sockfd, &response, sizeof(struct Response), 0) < 0) {
            fprintf(stderr, "Error: get server Login or Sign up message failed.\n");
            perror("recv");
            return -1;
        }
        sprintf(server_message, response.server_message);
        printf("%s", server_message);

        /*
         * Break loop if login or sign up is successful.
         */
        if ((strcmp(server_message, login_success) == 0) || (strcmp(server_message, sign_up_success) == 0)) {
            break;
        }

        /*
         * Continue loop if login or sign up is failed.
         */
        if ((strcmp(server_message, login_failed) == 0) || (strcmp(server_message, sign_up_failed) == 0)) {
            continue;
        }

        /*
         * get user input
         */
        memset(client_message, 0, sizeof(client_message));
        if (fgets(client_message, sizeof(client_message), stdin) == NULL) {
            fprintf(stderr, "Error: user input for login or sign up failed.\n");
            perror("fgets");
            return -1;
        }

        /*
         * setting client message to response
         */
        memset(&response, 0, sizeof(struct Response));
        sprintf(response.client_message, client_message);

        /*
         * sending login or sign up reponse to server
         */
        if (send(sockfd, &response, sizeof(struct Response), 0) < 0) {
            fprintf(stderr, "Error: sending message to server for login or sign up failed.\n");
            perror("send");
            return -1;
        }
        
    }

    return 0;
}