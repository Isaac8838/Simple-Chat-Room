#include "../../include/client_handler.h"

static int groupSockInit(struct sockaddr_in *server, struct User *user);
static int closeHandler(int sockfd);

/*
 * Sending group message to user
 */
void *groupMessageHandler(void* arg) {
    pthread_detach(pthread_self());

    struct Response     res;
    struct sockaddr_in  server, client;
    struct User         *user = (struct User *)arg;
    socklen_t           sin_siz;
    char                query[BUFSIZ];
    int                 latest;
    int                 server_socket, client_socket;


    /*
     * initial socket
     */
    if ((server_socket = groupSockInit(&server, user)) < 0) {
        user->status = USER;
        user->group_id = -1;
        fprintf(stderr, "Error: initial group server failed.\n");
        pthread_exit(NULL);
    }


    /*
     * accepting client
     */
    if ((client_socket = accept(server_socket, (struct sockaddr *)&client, &sin_siz)) < 0) {
        user->status = USER;
        user->group_id = -1;
        fprintf(stderr, "Error: accepting group client failed.\n");
        perror("accept");
        pthread_exit(NULL);
    }


    /*
     * fetching message from database and sending to client.
     * if user status equal to USER, then finish handler.
     */
    MYSQL_RES *result;
    MYSQL_ROW rows;
    latest = 0;
    while (1) {

        if (user->status == USER) {
            memset(&res, 0, sizeof(res));
            if (closeHandler(client_socket) < 0) {
                fprintf(stderr, "Error: close handler failed.\n");
                break;
            }

            break;

        }

        memset(query, 0, sizeof(query));
        sprintf(query, "SELECT id, user_id, user_name, message FROM group_messages WHERE group_id = %d AND id > %d", user->group_id, latest);
        if (mysql_query(user->db, query) && mysql_errno(user->db)) {
            fprintf(stderr, "Error: querying group message failed: %s.\n", mysql_error(user->db));
            user->status = USER;
            user->group_id = -1;
            if (closeHandler(client_socket) < 0) {
                fprintf(stderr, "Error: close handler from query failed.\n");
                break;
            }
            break;
        }

        result = mysql_store_result(user->db);
        if (result == NULL && mysql_errno(user->db)) {
            fprintf(stderr, "Error: fetching group message result failed: %s.\n", mysql_error(user->db));
            user->status = USER;
            user->group_id = -1;
            if (closeHandler(client_socket) < 0) {
                fprintf(stderr, "Error: close handler from result failed.\n");
                break;
            }
            break;
        }
        
        while ((rows = mysql_fetch_row(result))) {
            memset(&res, 0, sizeof(res));
            latest = atoi(rows[0]);
            res.user_id = atoi(rows[1]);
            sprintf(res.message.names, rows[2]);
            sprintf(res.message.messages, rows[3]);
            if (send(client_socket, &res, sizeof(res), 0) < 0) {
                fprintf(stderr, "Error: sending group message failed.\n");
                perror("send");
                close(server_socket);
                close(client_socket);
                pthread_exit(NULL);
            }

            usleep(1);
        }
        mysql_free_result(result);

        sleep(1);
    }

    close(server_socket);
    close(client_socket);
    pthread_exit(NULL);
}

/*
 * Initialize group socket
 * return -1 for error, otherwise 0.
 */
static int groupSockInit(struct sockaddr_in *server, struct User *user) {
    int sockfd;

    memset(&(*server), 0, sizeof(struct sockaddr_in));
    server->sin_family = PF_INET;
    server->sin_port = htons(PORT + user->user_id);
    inet_aton("127.0.0.1", &(server->sin_addr));

    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error: creating group server socket failed.\n");
        perror("socket");
        return -1;
    }

    if (bind(sockfd, (struct sockaddr *)&(*server), sizeof(*server)) < 0) {
        fprintf(stderr, "Error: binding group server address and port failed.\n");
        perror("bind");
        return -1;
    }

    if (listen(sockfd, 100) < 0) {
        fprintf(stderr, "Error: listening group server failed.\n");
        perror("listen");
        return -1;
    }

    return sockfd;
}

/*
 * send close message to user to close handler.
 * return -1 for error, otherwise 0.
 */
static int closeHandler(int sockfd) {
    struct Response res;

    memset(&res, 0, sizeof(res));
    sprintf(res.server_message, "close");           
    if (send(sockfd, &res, sizeof(res), 0) < 0) {
        fprintf(stderr, "Error: sending group close message failed.\n");
        perror("send");
        return -1;
    }

    return 0;
}