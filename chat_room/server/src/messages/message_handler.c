#include "../../include/client_handler.h"

/*
 * Handling user message and store in group message table.
 */
void messageHandler(struct User *user, int group_id, char *message) {
    char query[BUFSIZ];
    struct Response res;

    /*
     * If user doesn't have join a group
     * ask user join first.
     */
    if (user->status != GROUP) {
        
        memset(res.server_message, 0, sizeof(res.server_message));
        sprintf(res.server_message, "You need to join a group!\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(struct Response), 0) < 0) {
            fprintf(stderr, "Error: sending join group first message failed.\n");
            perror("send");
            return;
        }

    }

    /*
     * Insert message to group message table.
     * If failed, send failed message to client.
     */
    memset(query, 0, sizeof(query));
    sprintf(query, "INSERT INTO group_messages(user_id, user_name, group_id, message) VALUES (%d, '%s', %d, '%s')", user->user_id, user->name, group_id, message);
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: inserting message to group messages falied: %s.\n", mysql_error(user->db));
        
        memset(res.server_message, 0, sizeof(res.server_message));
        sprintf(res.server_message, "Couldn't send message to group.\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(struct Response), 0) < 0) {
            fprintf(stderr, "Error: sending fail message to send message to group failed.\n");
            perror("send");
            return;
        }
        
        return;
    }

    return;

}