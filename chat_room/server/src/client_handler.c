#include "../include/client_handler.h"

static int databaseConnection(MYSQL **db);
static int LoginOrSignUpHandler(struct User *user);
static void commandParser(struct Request *req, struct Command *cmd);


/*
 * A handler for handling client requests
 * Including commands and message.
 * 
 * When client logout
 * it exit by it self without join.
 * 
 * If error happend, client handler will end the service.
 */
void* clientHandler(void* arg) {
    pthread_detach(pthread_self());

    struct User user = *((struct User *)arg);
    free(arg);
    has_registered[user.id] = 1;
    
    /*
     * Connecting to database
     */
    if (databaseConnection(&user.db) < 0) {
        fprintf(stderr, "Error: failed to connect to database.\n");
        has_registered[user.id] = 0;
        User_online--;
        pthread_exit(NULL);
    }

    /*
     * Handling user login or sign up
     */
    if (LoginOrSignUpHandler(&user) < 0) {
        fprintf(stderr, "Error: login or sign up failed.\n");
        has_registered[user.id] = 0;
        User_online--;
        pthread_exit(NULL);
    }

    struct Request  req;
    struct Response res;
    struct Command  cmd;
    int             len;

    while (1) {

        /*
         * receiving request from user
         */
        if (recv(user.sockfd, &req, sizeof(struct Request), 0) < 0) {
            fpritnf(stderr, "Error: receiving user request from client handler failed.\n");
            perror("recv");
            break;
        }
        len = strlen(req.request);
        req.request[len - 1] = '\0';

        /*
         * Directing request to command handler or message handler.
         */
        if (req.request[0] == '!') {
            commandParser(&req, &cmd);
            commandHandler(&user, &cmd);
        } else {
            messageHandler(&user, req.request);
        }

    }

    has_registered[user.id] = 0;
    pthread_mutex_lock(&mutex);
    User_online--;
    pthread_mutex_unlock(&mutex);
    pthread_exit(NULL);
}


/*
 * Connecting to database
 * If no error, returning 0 else -1
 */
static int databaseConnection(MYSQL **db) {
    *db = mysql_init(NULL);

    if (*db == NULL) {
        fprintf(stderr, "Error: initializing db failed: %s.\n", mysql_error(*db));
        mysql_close(*db);
        return -1;
    }

    if (mysql_real_connect(*db, "host.docker.internal", "root", "password", "chat_room", 0, NULL, 0) == NULL) {
        fprintf(stderr, "Error: connecting to db server failed: %s.\n", mysql_error(*db));
        mysql_close(*db);
        return -1;
    }

    return 0;
}

/*
 * Handling user login or sign up
 * 1 is for login, 2 is for sign up
 * If no error, returning 0 else -1
 */
static int LoginOrSignUpHandler(struct User *user) {
    int len;
    struct Response response;

    while (1) {

        /*
         * 1 for Login, 2 for Sign up.
         */
        memset(response.server_message, 0, sizeof(response.server_message));
        response.method = LOGIN_OR_SIGNUP;
        sprintf(response.server_message, "Login or Sign up [1/2]? ");
        if (send(user->sockfd, &response, sizeof(struct Response), 0) < 0) {
            fprintf(stderr, "Error: login or sign up message failed.\n");
            perror("send");
            return -1;
        }

        memset(response.client_message, 0, sizeof(response.client_message));
        if (recv(user->sockfd, &response, sizeof(struct Response), 0) < 0) {
            fprintf(stderr, "Error: receiving Login or Sign up selection failed.\n");
            perror("recv");
            return -1;
        }
        len = strlen(response.client_message);
        response.client_message[len - 1] = '\0';

        /*
         * Handling client selection, Login or Sign up.
         */
        if (strcmp(response.client_message, "1") == 0) {
            int status;
            if ((status = login(&(*user))) < 0) {
                fprintf(stderr, "Error: login failed.\n");
                return -1;
            } else if (status == 1) {
                continue;
            } else {
                break;
            }
        } else if (strcmp(response.client_message, "2") == 0) {
            int status;
            if ((status = signUp(&(*user))) < 0) {
                fprintf(stderr, "Error: Sign up failed.\n");
                return -1;
            } else if (status == 1) {
                continue;
            } else {
                break;
            }
        }
    }

    return 0;
}

/*
 * Parsing user request to command structure.
 */
static void commandParser(struct Request *req, struct Command *cmd) {
    char type[BUFSIZ];
    memset(type, 0, sizeof(type));

    if (sscanf(req->request, "!%s %s", type, cmd->arg) == 1) {
        sprintf(cmd->arg, "none");
    }

    /*
     * Assigning type value accroding to user request
     * If no request doesn't match existing command
     * then assign command not found type.
     */
    if (strcmp(type, "create_group") == 0) {
        cmd->type = CREATE_GROUP;
    } else if (strcmp(type, "list_group") == 0) {
        cmd->type = LIST_GROUP;
    } else if (strcmp(type, "join_group") == 0) {
        cmd->type = JOIN_GROUP;
    } else if (strcmp(type, "logout") == 0) {
        cmd->type = LOGOUT;
    } else if (strcmp(type, "leave_group") == 0) {
        cmd->type = LEAVE_GROUP;
    } else if (strcmp(type, "list_user") == 0) {
        cmd->type = LIST_USER;
    } else if (strcmp(type, "invite") == 0) {
        cmd->type = INVITE;
    } else if (strcmp(type, "delete_group") == 0) {
        cmd->type = DELETE_GROUP;
    } else if (strcmp(type, "list_mail") == 0) {
        cmd->type = LIST_MAIL;
    } else if (strcmp(type, "mailto") == 0) {
        cmd->type = MAILTO;
    } else if (strcmp(type, "help") == 0) {
        cmd->type = HELP;
    } else {
        cmd->type = COMMAND_NOT_FOUND;
    }
}
