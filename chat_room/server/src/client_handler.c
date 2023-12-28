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
    while (1) {
        int status;
        if ((status = LoginOrSignUpHandler(&user)) < 0) {
            fprintf(stderr, "Error: login or sign up failed.\n");
            has_registered[user.id] = 0;
            User_online--;
            pthread_exit(NULL);
        } else if (status == 1) {
            continue;
        } else {
            break;
        }
    }
    

    struct Request  req;
    // struct Response res;
    struct Command  cmd;
    int             len;
    int             status;

    while (1) {

        /*
         * receiving request from user
         */
        if (recv(user.sockfd, &req, sizeof(struct Request), 0) < 0) {
            fprintf(stderr, "Error: receiving user request from client handler failed.\n");
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
            
            if ((status = commandHandler(&user, &cmd)) < 0) {
                fprintf(stderr, "Error: handling command failed.\n");
                break;
            } else if (status > 0) {
                break;
            }

        } else {
            if (messageHandler(&user, user.group_id, req.request) < 0) {
                fprintf(stderr, "Error: Handling client message failed.\n");
                break;
            }
        }

    }

    has_registered[user.id] = 0;
    pthread_mutex_lock(&mutex);
    User_online--;
    pthread_mutex_unlock(&mutex);
    mysql_close(user.db);
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
    struct Request  request;


    /*
        * 1 for Login, 2 for Sign up.
        */
    memset(&request, 0, sizeof(struct Request));
    sprintf(request.request, "Login or Sign up [1/2]? ");
    if (send(user->sockfd, &request, sizeof(request), 0) < 0) {
        fprintf(stderr, "Error: login or sign up message failed.\n");
        perror("send");
        return -1;
    }

    memset(&response, 0, sizeof(struct Response));
    if (recv(user->sockfd, &response, sizeof(struct Response), 0) < 0) {
        fprintf(stderr, "Error: receiving Login or Sign up selection failed.\n");
        perror("recv");
        return -1;
    }
    len = strlen(response.client_message);
    if (len > 0 && response.client_message[len - 1] == '\n') {
        response.client_message[len - 1] = '\0';
    }

    /*
        * Handling client selection, Login or Sign up.
        */
    if (strcmp(response.client_message, "1") == 0) {

        memset(&response, 0, sizeof(struct Response));
        sprintf(response.server_message, "valid");
        if (send(user->sockfd, &response, sizeof(response), 0) < 0) {
            fprintf(stderr, "Error: sending vaild command to user failed.\n");
            perror("send");
            return -1;
        }

        int status;
        if ((status = login(&(*user))) < 0) {
            fprintf(stderr, "Error: login failed.\n");
            return -1;
        } else if (status == 1) {
            return 1;
        } else {
            return 0;
        }
    } else if (strcmp(response.client_message, "2") == 0) {

        memset(&response, 0, sizeof(struct Response));
        sprintf(response.server_message, "valid");
        if (send(user->sockfd, &response, sizeof(response), 0) < 0) {
            fprintf(stderr, "Error: sending invaild command to user failed.\n");
            perror("send");
            return -1;
        }

        int status;
        if ((status = signUp(&(*user))) < 0) {
            fprintf(stderr, "Error: Sign up failed.\n");
            return -1;
        } else if (status == 1) {
            return 1;
        } else {
            return 0;
        }
    } else {
        memset(&response, 0, sizeof(struct Response));
        sprintf(response.server_message, "invalid");
        if (send(user->sockfd, &response, sizeof(response), 0) < 0) {
            fprintf(stderr, "Error: sending invaild command to user failed.\n");
            perror("send");
            return -1;
        }
        return 1;
    }
    

    return -1;
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
