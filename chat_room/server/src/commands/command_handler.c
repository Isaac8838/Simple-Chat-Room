#include "../../include/client_handler.h"

static int createGroupHandler(struct User *user, char *group_name);
static int listGroupHandler(struct User *user, char *arg);

void commandHandler(struct User *user, struct Command *cmd) {
    struct Response res;

    if (cmd->type == CREATE_GROUP) {
        if (createGroupHandler(&(*user), cmd->arg) < 0) {
            fprintf(stderr, "Error: creating a new group failed.\n");

            /*
             * Sending fail creating group message to client.
             */
            memset(res.server_message, 0, sizeof(res.server_message));
            sprintf(res.server_message, "Couldn't create a group. The group name might be registered. Please use list_group for checking.");
            res.method = SERVER_MESSAGE;
            if (send(user->sockfd, &res, sizeof(struct Response), 0) < 0) {
                fprintf(stderr, "Error: sending failed to creating group message failed.\n");
                perror("send");
                return;
            }

            return;

        } else {

            /*
             * Sending  successful creating group message to client.
             */
            memset(res.server_message, 0, sizeof(res.server_message));
            sprintf(res.server_message, "group %s creates successfully\n");
            res.method = SERVER_MESSAGE;
            if (send(user->sockfd, &res, sizeof(struct Response), 0) < 0) {
                fprintf(stderr, "Error: sending successful creating group message failed.\n");
                perror("send");
                return;
            }

        }
    } else if (cmd->type == LIST_GROUP) {
        if (listGroupHandler(&user, cmd->arg) < 0) {
            fprintf(stderr, "Error: listing group is failed.\n");
            
            /*
             * Sending fail listing group message
             */
            memset(res.server_message, 0, sizeof(res.server_message));
            sprintf(res.server_message, "Couldn't list groups.\n");
            res.method = SERVER_MESSAGE;
            if (send(user->sockfd, &res, sizeof(struct Response), 0) < 0) {
                fprintf(stderr, "Error: sending listing groups message failed.\n");
                perror("send");
                return;
            }

            return;

        }
    }

    return;
}

/*
 * Creating a group accroding to group name
 * return -1 for error, otherwise 0
 */
static int createGroupHandler(struct User *user, char *group_name) {
    char        query[BUFSIZ];
    int         group_id;
    MYSQL_RES   *result;
    MYSQL_ROW   row;
    memset(query, 0, sizeof(query));

    /*
     * Updating group_lists table with new group
     * and get the group id.
     */
    memset(query, 0, sizeof(query));
    sprintf(query, "INSERT INTO group_lists(group_name, user_id, owner) VALUES ('%s', %d, '%s'); "
                   "SELECT LAST_INSERT_ID();", group_name, user->user_id, user->name);
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: inserting new group to group_lists table failed: %s.\n", mysql_error(user->db));
        return -1;
    }
    
    result = mysql_store_result(user->db);
    if (result == NULL) {
        fprintf(stderr, "Error: fetching result from group_lists is falied: %s.\n", mysql_error(user->db));
        return -1;
    }

    row = mysql_fetch_row(result);
    if (mysql_errno(user->db) != 0) {
        fprintf(stderr, "Error: fetching row from group lists failed: %s\n", mysql_error(user->db));
        return -1;
    }
    group_id = atoi(row[0]);
    
    /*
     * Inserting a new row with user name, group name and group id.
     */
    memset(query, 0, sizeof(query));
    sprintf(query, "INSERT INTO user_groups(user_id, user_name, group_id) VALUES(%d, '%s', %d)", user->user_id, user->name, group_id);
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: creating a new group table failed: %s.\n", mysql_error(user->db));

        /*
         * If inserting a new row with user name and group name failed
         * then delete group from group lists table.
         */
        memset(query, 0, sizeof(query));
        sprintf(query, "DELETE FROM group_lists WHERE id = %d", group_id);
        if (mysql_query(user->db, query)) {
            fprintf(stderr, "ERROR: deleting group from group lists table failed: %s.\n", mysql_error(user->db));
            return -1;
        }

        return -1;
    }

    return 0;
}

/*
 * Listing groups from group lists table
 * return -1 for error, otherwise 0.
 */
static int listGroupHandler(struct User *user, char *arg) {
    struct Response res;
    char            query_count[BUFSIZ];
    char            query_list[BUFSIZ];
    int             number_of_groups;
    int             i;
    MYSQL_RES       *result;
    MYSQL_ROW       rows;

    /*
     * Getting the number of groups
     * If with "self" argument
     * fetching the group that have joined
     * otherwise fetching all of the groups
     */
    memset(query_count, 0, sizeof(query_count));
    memset(query_list, 0, sizeof(query_list));
    if (strcmp(arg, "self") == 0) {
        sprintf(query_count, "SELECT COUNT(*) FROM user_groups"
                       "JOIN group_lists ON group_lists.id = user_groups.group_id"
                       "WHERE user_id = %d;", user->user_id);
        sprintf(query_count, "SELECT COUNT(group_name, owner) FROM user_groups"
                       "JOIN group_lists ON group_lists.id = user_groups.group_id"
                       "WHERE user_id = %d;", user->user_id);
    } else if (strcmp(arg, "none") == 0) {
        sprintf(query_count, "SELECT COUNT(*), owner FROM group_lists");
        sprintf(query_count, "SELECT group_name, owner FROM group_lists");
    }

    if (mysql_query(user->db, query_count)) {
        fprintf(stderr, "Error: getting number of rows in group lists failed: %s.\n", user->db);
        return -1;
    }

    result = mysql_store_result(user->db);
    if (result == NULL) {
        fprintf(stderr, "Error: retrieving result set from group_lists failed: %s.\n", mysql_error(user->db));
        return -1;
    }

    rows = mysql_fetch_row(result);
    if (rows == NULL) {
        fprintf(stderr, "Error: fetching number of group lists failed: %s.\n", mysql_error(user->db));
        return -1;
    }

    number_of_groups = atoi(rows[0]);
    res.message = malloc(sizeof(struct Message) * number_of_groups);

    /*
     * Fetching groups from group lists and send to user
     */
    if (mysql_query(user->db, query_list)) {
        fprintf(stderr, "Error: fetching groups from group lists failed: %s.\n", mysql_error(user->db));
        return -1;
    }

    result = mysql_store_result(user->db);
    if (result == NULL) {
        fprintf(stderr, "Error: fetching result set from group lists failed: %s.\n", mysql_error(user->db));
        return -1;
    }

    i = 0;
    while ((rows = mysql_fetch_row(result))) {

        if (mysql_errno(user->db) != 0) {
            fprintf(stderr, "Error: fetching group from group lists failed: %s.\n", mysql_error(user->db));
            return -1;
        }

        memset(res.message[i].groups, 0, sizeof(res.message[i].groups));
        sprintf(res.message[i].groups, "%s", rows[0]);
        sprintf(res.message[i].names, "%s", rows[1]);
        i++;
        
    }

    /*
     * Sending group lists response to user
     */
    if (send(user->sockfd, &res, sizeof(struct Response), 0) < 0) {
        fprintf(stderr, "Error: sending response from group lists is failed.\n");
        perror("send");
        return -1;
    }

    return 0;

}