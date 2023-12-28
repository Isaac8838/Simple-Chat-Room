#include "../../include/client_handler.h"

/*
 * Handling user message and store in group message table.
 */
int messageHandler(struct User *user, int group_id, char *message) {
    char            query[BUFSIZ];
    struct Response res;
    MYSQL_RES       *result;
    MYSQL_ROW       row;

    /*
     * If user doesn't have join a group
     * ask user join first.
     */
    if (user->status != GROUP) {
        
        memset(&res, 0, sizeof(struct Response));
        sprintf(res.server_message, "You need to join a group!\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending join group first message failed.\n");
            perror("send");
            return -1;
        }

        return 0;

    }

    /*
     * If group has been deleted
     * set user to USER mode
     */
    memset(query, 0, sizeof(query));
    sprintf(query, "SELECT * FROM group_lists WHERE id = %d;", group_id);
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: searching group from group lists in message handler failed: %s.\n", mysql_error(user->db));
        return -1;
    }
    result = mysql_store_result(user->db);
    if (result == NULL) {
        fprintf(stderr, "Error: fetching result from message handler failed: %s.\n", mysql_error(user->db));
        return -1;
    }
    row = mysql_fetch_row(result);
    mysql_free_result(result);
    if (row == NULL) {
        user->status = USER;
        user->group_id = -1;
        memset(&res, 0, sizeof(res));
        sprintf(res.server_message, "Group has been deleted.\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending group has been deleted from message handler failed.\n");
            perror("send");
            return -1;
        }

        return 0;
    }
    

    /*
     * Insert message to group message table.
     * If failed, send failed message to client.
     */
    memset(query, 0, sizeof(query));
    sprintf(query, "INSERT INTO group_messages(user_id, user_name, group_id, message) VALUES (%d, '%s', %d, '%s')", user->user_id, user->name, group_id, message);
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: inserting message to group messages falied: %s.\n", mysql_error(user->db));
        
        memset(&res, 0, sizeof(struct Response));
        sprintf(res.server_message, "Couldn't send message to group.\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending fail message to send message to group failed.\n");
            perror("send");
            return -1;
        }
        
        return -1;
    }

    /*
     * sending message method
     */
    memset(&res, 0, sizeof(res));
    res.method = MESSAGE;
    if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
        fprintf(stderr, "Error: sending message method to group failed.\n");
        perror("send");
        return -1;
    }

    return 0;

}