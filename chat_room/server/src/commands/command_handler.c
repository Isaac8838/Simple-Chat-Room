#include "../../include/client_handler.h"

static int createGroupHandler(struct User *user, char *group_name);
static int listGroupHandler(struct User *user, char *arg);
static int logout(struct User *user);
static int joinGroup(struct User *user, char *group_name);
static int leaveGroup(struct User *user);
static int mailto(struct User *user, int receiver_id, char *receiver, char *group_name);
static int invite(struct User *user, char *name);
static int listMail(struct User *user);
static int deleteGroup(struct User *user);

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

            return 0;

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
        int status;
        if ((status = listGroupHandler(&(*user), cmd->arg)) < 0) {
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

        } else if (status == 1) {
            memset(&res, 0, sizeof(res));
            res.method = ERROR;
            if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
                fprintf(stderr, "Error: sending error method from join group failed.\n");
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
        memset(&res, 0, sizeof(struct Response));
        sprintf(res.server_message, "command not found.\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending command not found message failed.\n");
            return -1;
        }
    } else if (cmd->type == JOIN_GROUP) {
        int status;
        if ((status = joinGroup(&(*user), cmd->arg)) < 0) {
            fprintf(stderr, "Error: joining group failed.\n");

            memset(&res, 0, sizeof(res));
            res.method = ERROR;
            if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
                fprintf(stderr, "Error: sending error method from join group failed.\n");
                perror("send");
                return -1;
            }

            return -1;
        } else if (status == 1) {
            memset(&res, 0, sizeof(res));
            sprintf(res.server_message, "You haven't invited to the group.\nSending joining request to group owner.\n");
            res.method = SERVER_MESSAGE;
            if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
                fprintf(stderr, "Error: send joinGroup error message failed.\n");
                perror("send");
                return -1;
            }
            return 0;
        }
    } else if (cmd->type == LEAVE_GROUP) {
        if (leaveGroup(&(*user)) < 0) {
            fprintf(stderr, "Error: leaving group failed.\n");
            return -1;
        }
    } else if (cmd->type == INVITE) {
        if (invite(&(*user), cmd->arg) < 0) {
            fprintf(stderr, "Error: invite failed.\n");

            memset(&res, 0, sizeof(res));
            res.method = ERROR;
            if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
                fprintf(stderr, "Error: sending error method from join group failed.\n");
                perror("send");
                return -1;
            }

            return -1;
        }
    } else if (cmd->type == LIST_MAIL) {
        if (listMail(&(*user)) < 0) {
            fprintf(stderr, "Error: listing mails failed.\n");
            
            memset(&res, 0, sizeof(res));
            res.method = ERROR;
            if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
                fprintf(stderr, "Error: sending error method from join group failed.\n");
                perror("send");
                return -1;
            }

            return -1;
        }
    } else if (cmd->type == DELETE_GROUP) {
        int status;
        if ((status = deleteGroup(&(*user))) < 0) {
            fprintf(stderr, "Error: deleting group failed.\n");
            
            memset(&res, 0, sizeof(res));
            res.method = ERROR;
            if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
                fprintf(stderr, "Error: sending error method from join group failed.\n");
                perror("send");
                return -1;
            }
            
            return -1;
        } else if (status == 1) {
            memset(&res, 0, sizeof(res));
            sprintf(res.server_message, "Deleting group failed.\nMake sure you are group owner.\nPlease try again.\n");
            res.method = SERVER_MESSAGE;
            if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
                fprintf(stderr, "Error: sending deleting failed failed.\n");
                perror("send");
                return -1;
            }
            return 0;
        }
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
    sprintf(query, "INSERT INTO user_groups(user_id, user_name, group_id, group_name) VALUES(%d, '%s', %d, '%s')", user->user_id, user->name, group_id, group_name);
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
 * return -1 for error, 1 for unexpected sql error, otherwise 0.
 */
static int listGroupHandler(struct User *user, char *arg) {
    struct Response res;
    char            query[BUFSIZ];
    MYSQL_RES       *result;
    MYSQL_ROW       rows;

    /*
     * Getting the number of groups
     * If with "self" argument
     * fetching the group that have joined
     * otherwise fetching all of the groups
     */
    memset(query, 0, sizeof(query));
    if (strcmp(arg, "self") == 0) {
        sprintf(query, "SELECT user_groups.group_name, owner FROM user_groups "
                       "JOIN group_lists ON group_lists.id = user_groups.group_id "
                       "WHERE user_id = %d;", user->user_id);
    } else if (strcmp(arg, "none") == 0) {
        sprintf(query, "SELECT group_name, owner FROM group_lists;");
    } else {
        memset(&res, 0, sizeof(res));
        sprintf(res.server_message, "Invaild argument for list group.\nPlease try again.\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending invalid argument in list group failed.\n");
            perror("send");
            return -1;
        }
        return 0;
    }

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
     * Fetching groups from group lists and send to user
     */
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: fetching groups from group lists failed: %s.\n", mysql_error(user->db));
        return 1;
    }

    result = mysql_store_result(user->db);
    if (result == NULL) {
        fprintf(stderr, "Error: fetching result set from group lists failed: %s.\n", mysql_error(user->db));
        return 1;
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
        return 1;
    }

    /*
     * sending group list to user.
     */
    while ((rows = mysql_fetch_row(result))) {

        if (mysql_errno(user->db) != 0) {
            fprintf(stderr, "Error: fetching group from group lists failed: %s.\n", mysql_error(user->db));
            return 1;
        }

        memset(&res, 0, sizeof(struct Response));
        sprintf(res.message.groups, rows[0]);
        sprintf(res.message.names, rows[1]);
        
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending response from group lists is failed.\n");
            perror("send");
            return -1;
        }

        usleep(1);
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

    sleep(1);

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

/*
 * Joining a group
 * return -1 for error, 1 for group not found, otherwise 0.
 */
static int joinGroup(struct User *user, char *group_name) {
    struct Response     res;
    MYSQL_RES           *result;
    MYSQL_ROW           row;
    char                query[BUFSIZ];
    int                 owner_id;
    char                owner[255];
    pthread_t           thread;

    /*
     * Lack of group name.
     */
    if (strcmp(group_name, "none") == 0) {
        memset(&res, 0, sizeof(res));
        sprintf(res.server_message, "Please enter group name.\nUsage: !join_group name_of_group\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending join group, group name is none failed.\n");
            perror("send");
            return -1;
        }

        return 0;
    }

    /*
     * If user already in a group
     * Ask then leave group first.
     */
    if (user->status != USER) {
        memset(&res, 0, sizeof(res));
        sprintf(res.server_message, "Please leave group first.\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending join group, group name is none failed.\n");
            perror("send");
            return -1;
        }

        return 0;
    }


    /*
     * If group doesn't exist in group list
     * Ask user to enter a correct group name.
     */
    memset(query, 0, sizeof(query));
    sprintf(query, "SELECT owner_id, owner FROM group_lists WHERE group_name = '%s'", group_name);
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: searching group from group lists in join group command failed.\n");
        return -1;
    }

    result = mysql_store_result(user->db);
    if (result == NULL) {
        fprintf(stderr, "Error: fetching result from group lists in join group failed.\n");
        return -1;
    }

    row = mysql_fetch_row(result);
    mysql_free_result(result);
    if (row == NULL) {
        memset(&res, 0, sizeof(res));
        sprintf(res.server_message, "Group not exist.\nPlease try again.\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending group doesn't exist from join group failed.\n");
            perror("send");
            return -1;
        }
        return 0;
    } else {
        owner_id = atoi(row[0]);
        sprintf(owner, row[1]);
    }
    

    /*
     * Checking whether the group is exist?
     */
    memset(query, 0, sizeof(query));
    sprintf(query, "SELECT group_id FROM user_groups WHERE user_id = %d AND group_name = '%s';", user->user_id, group_name);
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: searching group from group lists failed: %s.\n", mysql_error(user->db));
        return -1;
    }
    
    result = mysql_store_result(user->db);
    if (result == NULL) {
        fprintf(stderr, "Error: fetching group name from group lists result failed: %s.\n", mysql_error(user->db));
        return -1;
    }

    row = mysql_fetch_row(result);
    mysql_free_result(result);
    if (row == NULL) {

        if (mailto(&(*user), owner_id, owner, group_name) < 0) {
            fprintf(stderr, "Error: mailto failed.\n");
            return -1;
        }

        return 1;
    }

    (&(*user))->status = GROUP;
    (&(*user))->group_id = atoi(row[0]);

    if (pthread_create(&thread, NULL, groupMessageHandler, (void *)user)) {
        fprintf(stderr, "create group message handler failed.\n");
        return -1;
    }

    /*
     * Sending join group method
     */
    memset(&res, 0, sizeof(res));
    res.method = JOIN_GROUP;
    res.user_id = user->user_id;
    if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
        fprintf(stderr, "Error: sending join group method failed.\n");
        perror("send");
        return -1;
    }

    return 0;
}

/*
 * leaving a group
 */
static int leaveGroup(struct User *user) {
    struct Response res;

    /*
     * This command will only work when user join a group.
     * which means in GROUP mode.
     */
    if (user->status == USER) {
        memset(&res, 0, sizeof(res));
        sprintf(res.server_message, "You Don't have joined a group!\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending leave group successful message failed.\n");
            perror("send");
            return -1;
        }
        return 0;
    }

    /*
     * Changing user status and group id to USER mode
     * and sending message to client.
     */
    user->status = USER;
    user->group_id = -1;

    memset(&res, 0, sizeof(res));
    sprintf(res.server_message, "Leaving group successfully!\n");
    res.method = SERVER_MESSAGE;
    if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
        fprintf(stderr, "Error: sending leave group successful message failed: %s.\n", mysql_error(user->db));
        perror("send");
        return -1;
    }

    return 0;
}

/*
 * Mailing message to receiver from sender
 * return -1 for error, otherwise 0.
 */
static int mailto(struct User *user, int receiver_id, char *receiver, char *group_name) {
    char query[BUFSIZ + BUFSIZ];
    char messages[BUFSIZ];

    memset(query, 0, sizeof(query));
    memset(messages, 0, sizeof(messages));
    sprintf(messages, "%s wants to join %s", user->name, group_name);
    sprintf(query, "INSERT INTO mails(receiver_id, receiver, sender_id, sender, messages) VALUES(%d, '%s', %d, '%s', '%s');", receiver_id, receiver, user->user_id, user->name, messages);
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: inserting mail to mails database failed: %s.\n", mysql_error(user->db));
        return -1;
    }

    return 0;
}

/*
 * Inviting user to join the group
 */
static int invite(struct User *user, char *name) {
    struct Response res;
    int             name_id = -1;
    char            group[255];
    char            query[BUFSIZ];
    MYSQL_RES       *result;
    MYSQL_ROW       row;

    if (user->status != GROUP) {
       memset(&res, 0, sizeof(res));
        sprintf(res.server_message, "Please join the group that you want to invite");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending join group first from invite handler failed.\n");
            perror("send");
            return -1;
        }
    }

    /*
     * Searching for user id
     */
    memset(query, 0, sizeof(query));
    sprintf(query, "SELECT id FROM users WHERE name = '%s';", name);
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: searching user id from users in invite handler failed: %s.\n", mysql_error(user->db));
        return -1;
    }

    result = mysql_store_result(user->db);
    if (result == NULL) {
        fprintf(stderr, "Error: fetching result from users in invite handler failed: %s.\n", mysql_error(user->db));
        return -1;
    }

    row = mysql_fetch_row(result);
    mysql_free_result(result);
    if (row == NULL) {
        memset(&res, 0, sizeof(res));
        sprintf(res.server_message, "User can not be found.\nPlease try again.\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending user cannot be found from invite handler failed.\n");
            perror("send");
            return -1;
        }

        return 0;
    }
    name_id = atoi(row[0]);

    /*
     * Searching for group name.
     */
    memset(query, 0, sizeof(query));
    sprintf(query, "SELECT group_name FROM group_lists WHERE id = %d AND owner_id = %d;", user->group_id, user->user_id);
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: searching group_id and owner_id from group_lists in invite handler failed: %s.\n", mysql_error(user->db));
        return -1;
    }

    result = mysql_store_result(user->db);
    if (result == NULL) {
        fprintf(stderr, "Error: fetching result from group_lists in invite handler failed: %s.\n", mysql_error(user->db));
        return -1;
    }

    row = mysql_fetch_row(result);
    mysql_free_result(result);
    if (row == NULL) {
        memset(&res, 0, sizeof(res));
        sprintf(res.server_message, "Join the group that you want to invite first.\nPlease try again.\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending group_lists cannot be found from invite handler failed.\n");
            perror("send");
            return -1;
        }

        return 0;
    }
    sprintf(group, row[0]);

    /*
     * inviting user to the group
     */
    memset(query, 0, sizeof(query));
    sprintf(query, "INSERT INTO user_groups(user_id, user_name, group_id, group_name) VALUES(%d, '%s', %d, '%s')", name_id, name, user->group_id, group);
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: searching user id from users in invite handler failed: %s.\n", mysql_error(user->db));
        return -1;
    }

    memset(&res, 0, sizeof(res));
    sprintf(res.server_message, "You have invited %s to group %s successfully.\n", name, group);
    res.method = SERVER_MESSAGE;
    if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
        fprintf(stderr, "Error: sending invite successful message failed.\n");
        return -1;
    }

    return 0;
    
}


/*
 * Listing user mails
 * return -1 for error, otherwise 0.
 */
static int listMail(struct User *user) {
    struct Response res;
    char            query[BUFSIZ];
    MYSQL_RES       *result;
    MYSQL_ROW       rows;

    /*
     * Sending list mails method to user
     */
    memset(&res, 0, sizeof(res));
    res.method = LIST_MAIL;
    if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
        fprintf(stderr, "Error: sending list mails failed.\n");
        perror("send");
        return -1;
    }

    /*
     * Fetching mails of user and sending it to client.
     */
    memset(&query, 0, sizeof(query));
    sprintf(query, "SELECT sender, messages FROM mails WHERE receiver_id = %d", user->user_id);
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: querying mails failed: %s.\n", mysql_error(user->db));
        return -1;
    }

    result = mysql_store_result(user->db);
    if (result == NULL) {
        fprintf(stderr, "Error: fetching mails result failed: %s.\n", mysql_error(user->db));
        return -1;
    }

    while ((rows = mysql_fetch_row(result))) {
        memset(&res, 0, sizeof(res));
        sprintf(res.message.names, rows[0]);
        sprintf(res.message.messages, rows[1]);
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending mails response to user failed.\n");
            perror("send");
            return -1;
        }
        usleep(1);
    }

    memset(&res, 0, sizeof(res));
    sprintf(res.server_message, "end");
    if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
        fprintf(stderr, "Error: sending list mail end failed.\n");
        return -1;
    }

    return 0;

}

/*
 * Deleting group
 * return -1 for error, otherwise 0.
 */
static int deleteGroup(struct User *user) {
    struct Response res;
    char            query[BUFSIZ];
    int             affected;

    /*
     * If user haven't joined a group
     * Ask user join group first.
     */
    if (user->status != GROUP) {
        memset(&res, 0, sizeof(res));
        sprintf(res.server_message, "You need to join a group to perfrom this action.\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending join group first from delete group failed.\n");
            perror("send");
            return -1;
        }
        return 0;
    }

    /*
     * Deleting group from group lists.
     * return -1 for error, 1 for not owner, otherwise 0.
     */
    memset(query, 0, sizeof(query));
    sprintf(query, "DELETE FROM group_lists WHERE id = %d AND owner_id = %d;", user->group_id, user->user_id);
    if (mysql_query(user->db, query) && mysql_errno(user->db)) {
        fprintf(stderr, "Error: deleting group from group_lists failed: %s.\n", mysql_error(user->db));
        return -1;
    }
    affected = mysql_affected_rows(user->db);
    if (affected == 0) {
        return 1;
    }
    

    memset(&res, 0, sizeof(res));
    sprintf(res.server_message, "Deleting group successfully!\n");
    res.method = SERVER_MESSAGE;
    if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
        fprintf(stderr, "Error: sending deleting successful message failed.\n");
        perror("send");
        return -1;
    }

    return 0;

}