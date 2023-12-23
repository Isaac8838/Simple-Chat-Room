#include "../include/client.h"
#include "../include/server_handler.h"
#include "../include/socket_conn.h"

static int LoginOrSignUpHandler(int sockfd);

int main() {
    int     client_socket;
    struct  sockaddr_in server;

    client_socket = sockConnection(&server);
    
    while (1) {
        int status;
        if ((status = LoginOrSignUpHandler(client_socket)) < 0) {
            fprintf(stderr, "Error: login or sign up failed.\n");
            exit(EXIT_FAILURE);
        } else if (status == 1) {
            continue;;
        } else {
            break;
        }
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
    struct Response response;
    struct Request  request;     
    const char      login_success[BUFSIZ] = "Login successfully!\n";
    const char      sign_up_success[BUFSIZ] = "Sign up successfully!\n";
    const char      login_failed[BUFSIZ] = "User doesn't exsit.\n";
    const char      sign_up_failed[BUFSIZ] = "User has been registered.\n";

    while (1) {

        /*
         * Receiving server response Login or Sign up selection.
         */
        memset(&request, 0, sizeof(struct Request));
        if (recv(sockfd, &request, sizeof(struct Request), 0) < 0) {
            fprintf(stderr, "Error: get server Login or Sign up message failed.\n");
            perror("recv");
            return -1;
        }
        printf("%s", request.request);
        fflush(stdout);

        /*
         * Responing to server user's selection
         */
        memset(&response, 0, sizeof(struct Response));
        if (fgets(response.client_message, sizeof(response.client_message), stdin) == NULL) {
            fprintf(stderr, "Error: user input for login or sign up failed.\n");
            perror("fgets");
            return -1;
        }

        if (send(sockfd, &response, sizeof(response), 0) < 0) {
            fprintf(stderr, "Error: sending server Login or Sign up selection message failed.\n");
            perror("send");
            return -1;
        }

        memset(&response, 0, sizeof(struct Response));
        if (recv(sockfd, &response, sizeof(struct Response), 0) < 0) {
            fprintf(stderr, "Error: get server valid login or sign up selection message failed.\n");
            perror("recv");
            return -1;
        }

        if (strcmp(response.server_message, "valid") != 0) {
            printf("invalid number, please try again.\n");
            fflush(stdout);
            return 1;
        }


        /*
         * Receiving user name message from server
         */
        memset(&request, 0, sizeof(struct Request));
        if (recv(sockfd, &request, sizeof(struct Request), 0) < 0) {
            fprintf(stderr, "Error: Receiving user name message failed.\n");
            perror("recv");
            return -1;
        }
        printf("%s", request.request);
        fflush(stdout);

        /*
         * Sending user name
         */
        memset(&response, 0, sizeof(struct Response));
        if (fgets(response.client_message, sizeof(response.client_message), stdin) == NULL) {
            fprintf(stderr, "Error: user input for name failed.\n");
            perror("fgets");
            return -1;
        }

        if (send(sockfd, &response, sizeof(response), 0) < 0) {
            fprintf(stderr, "Error: sending user name message failed.\n");
            perror("send");
            return -1;
        }

        /*
         * Receving password message
         */
        memset(&request, 0, sizeof(struct Request));
        if (recv(sockfd, &request, sizeof(struct Request), 0) < 0) {
            fprintf(stderr, "Error: Receiving user password message failed.\n");
            perror("recv");
            return -1;
        }
        printf("%s", request.request);
        fflush(stdout);

        /*
         * Sending user password
         */
        memset(&response, 0, sizeof(struct Response));
        if (fgets(response.client_message, sizeof(response.client_message), stdin) == NULL) {
            fprintf(stderr, "Error: user input for user password failed.\n");
            perror("fgets");
            return -1;
        }

        if (send(sockfd, &response, sizeof(response), 0) < 0) {
            fprintf(stderr, "Error: sending user password message failed.\n");
            perror("send");
            return -1;
        }

        /*
         * Receving server login or sign up successful message or failed message
         */
        memset(&response, 0, sizeof(struct Response));
        if (recv(sockfd, &response, sizeof(struct Response), 0) < 0) {
            fprintf(stderr, "Error: receivinging user server login or sign up successful or failed message failed.\n");
            perror("recv");
            return -1; 
        }
        printf("%s", response.server_message);
        fflush(stdout);

        /*
         * Break loop if login or sign up is successful.
         */
        if ((strcmp(response.server_message, login_success) == 0) || (strcmp(response.server_message, sign_up_success) == 0)) {
            return 0;
        }

        /*
         * Continue loop if login or sign up is failed.
         */
        if ((strcmp(response.server_message, login_failed) == 0) || (strcmp(response.server_message, sign_up_failed) == 0)) {
            return 1;
        }
        
    }

    return -1;
}