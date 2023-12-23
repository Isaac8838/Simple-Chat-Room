#include "../../include/client_handler.h"

static int createGroupHandler(struct User *user, char *group_name);
static int listGroupHandler(struct User *user, char *arg);
static int logout(struct User *user);

/*
 * Handling commands
 * return -1 for error
 * return 1 for logout
 * otherwise 0
 */
int commandHandler(struct User *user, struct Command *cmd) {
    struct Response res;

    if (cmd->type == CREATE_GROUP) {
        int status;
        if ((status = createGroupHandler(&(*user), cmd->arg)) < 0) {
            fprintf(stderr, "Error: creating a new group failed.\n");

            /*
             * Sending fail creating group message to client.
             */
            memset(&res, 0, sizeof(struct Response));
            sprintf(res.server_message, "Couldn't create a group. The group name might be registered or doesn't enter group name.\n"
                                        "Please use list_group for checking.\n");
            res.method = SERVER_MESSAGE;
            if (send(user->sockfd, &res, sizeof(struct Response), 0) < 0) {
                fprintf(stderr, "Error: sending failed to creating group message failed.\n");
                perror("send");
                return -1;
            }

            return -1;

        } else {

            /*
             * Sending  successful creating group message to client.
             */
            memset(&res, 0, sizeof(struct Response));
            sprintf(res.server_message, "group %s creates successfully\n", cmd->arg);
            res.method = SERVER_MESSAGE;
            if (send(user->sockfd, &res, sizeof(struct Response), 0) < 0) {
                fprintf(stderr, "Error: sending successful creating group message failed.\n");
                perror("send");
                return -1;
            }

        }
    } else if (cmd->type == LIST_GROUP) {
        if (listGroupHandler(&(*user), cmd->arg) < 0) {
            fprintf(stderr, "Error: listing group is failed.\n");
            
            /*
             * Sending fail listing group message
             */
            memset(&res, 0, sizeof(struct Response));
            sprintf(res.server_message, "Couldn't list groups.\n");
            res.method = SERVER_MESSAGE;
            if (send(user->sockfd, &res, sizeof(struct Response), 0) < 0) {
                fprintf(stderr, "Error: sending listing groups message failed.\n");
                perror("send");
                return -1;
            }

            return -1;

        }
    } else if (cmd->type == LOGOUT) {

        /*
         * handling logout request.
         */
        if (logout(&(*user)) < 0) {
            fprintf(stderr, "Error: logout request failed.\n");
            return -1;
        } else {
            return 1;
        }

    } else if (cmd->type == COMMAND_NOT_FOUND) {
        return 0;
    }

    return 0;
}

/*
 * Creating a group accroding to group name
 * return -1 for error, otherwise 0
 */
static int createGroupHandler(struct User *user, char *group_name) {
    char        query[BUFSIZ];
    int         group_id;
    memset(query, 0, sizeof(query));

    /*
     * if group name is none
     * ask user to input group name.
     */
    if (strcmp(group_name, "none") == 0) {
        return -1;
    }

    /*
     * Updating group_lists table with new group
     * and get the group id.
     */
    memset(query, 0, sizeof(query));
    sprintf(query, "INSERT INTO group_lists(group_name, owner_id, owner) VALUES ('%s', %d, '%s'); ", group_name, user->user_id, user->name);
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: inserting new group to group_lists table failed: %s.\n", mysql_error(user->db));
        return -1;
    }
    group_id = mysql_insert_id(user->db);
    
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
    char            query[BUFSIZ];
    MYSQL_RES       *result;
    MYSQL_ROW       rows;

    /*
     * Sending list group method
     */
    memset(&res, 0, sizeof(struct Response));
    res.method = LIST_GROUP;
    if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
        fprintf(stderr, "Error: sending group list method failed.\n");
        perror("send");
        return -1;
    }


    /*
     * Getting the number of groups
     * If with "self" argument
     * fetching the group that have joined
     * otherwise fetching all of the groups
     */
    memset(query, 0, sizeof(query));
    if (strcmp(arg, "self") == 0) {
        sprintf(query, "SELECT group_name, owner FROM user_groups "
                       "JOIN group_lists ON group_lists.id = user_groups.group_id "
                       "WHERE user_id = %d;", user->user_id);
    } else if (strcmp(arg, "none") == 0) {
        sprintf(query, "SELECT group_name, owner FROM group_lists;");
    }

    /*
     * Fetching groups from group lists and send to user
     */
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: fetching groups from group lists failed: %s.\n", mysql_error(user->db));
        return -1;
    }

    result = mysql_store_result(user->db);
    if (result == NULL) {
        fprintf(stderr, "Error: fetching result set from group lists failed: %s.\n", mysql_error(user->db));
        return -1;
    }

    /*
     * Infroming user listing group routine start.
     */
    memset(&res, 0, sizeof(struct Response));
    sprintf(res.server_message, "start list");
    res.method = LIST_GROUP;
    if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
        fprintf(stderr, "Error: sending start response from group lists is failed.\n");
        perror("send");
        return -1;
    }

    /*
     * sending group list to user.
     */
    while ((rows = mysql_fetch_row(result))) {

        if (mysql_errno(user->db) != 0) {
            fprintf(stderr, "Error: fetching group from group lists failed: %s.\n", mysql_error(user->db));
            return -1;
        }

        memset(&res, 0, sizeof(struct Response));
        sprintf(res.message.groups, rows[0]);
        sprintf(res.message.names, rows[1]);
        
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending response from group lists is failed.\n");
            perror("send");
            return -1;
        }

    }
    mysql_free_result(result);

    /*
     * informing user listing group end.
     */
    memset(&res, 0, sizeof(struct Response));
    sprintf(res.server_message, "end list");
    res.method = LIST_GROUP;
    if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
        fprintf(stderr, "Error: sending response from group lists is failed.\n");
        perror("send");
        return -1;
    }

    return 0;

}

/*
 * logout, set user status to USER in order to close group message handler
 * return -1 for error, otherwise 0.
 */
static int logout(struct User *user) {
    struct Response res;
    struct Request  req;
    user->status = USER;
    user->group_id = -1;

    /*
     * Sending logout method
     */
    memset(&res, 0, sizeof(struct Response));
    res.method = LOGOUT;
    if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
        fprintf(stderr, "Error: sending logout method failed.\n");
        perror("send");
        return -1;
    }

    /*
     * Sending logout successful message to user.
     */
    memset(&res, 0, sizeof(struct Response));
    sprintf(res.server_message, "You have successfully logout!.\n");
    res.method = LOGOUT;
    if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
        fprintf(stderr, "Error: sending logout successful message failed.\n");
        perror("send");
        return -1;
    }
    

    /*
     * Sending confirmation message to client and end the client handler.
     */
    memset(&req, 0, sizeof(struct Request));
    sprintf(req.request, "close");
    if (send(user->sockfd, &req, sizeof(req), 0) < 0) {
        fprintf(stderr, "Error: sending close message failed.\n");
        perror("send");
        return -1;
    }

    return 0;

}