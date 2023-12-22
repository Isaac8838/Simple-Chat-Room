#include "../include/client_handler.h"

/*
 * search user from database.
 * If user not exist, return 1.
 * If user has been found, update user struct.
 * return -1 for error, otherwise return 0.
 */
int login(struct User *user) {
    char            query[BUFSIZ];
    char            name[255];
    char            password[255];
    int             len;
    struct Response res;
    struct Request  req;
    memset(query, 0, sizeof(query));    
    memset(name, 0, sizeof(name));
    memset(password, 0, sizeof(password));

    /*
     * Get name from client
     */
    memset(&req, 0, sizeof(struct Request));
    sprintf(req.request, "name: ");
    if (send(user->sockfd, &req, sizeof(req), 0) < 0) {
        fprintf(stderr, "Error: sending name message to user failed.\n");
        perror("send");
        return -1;
    }

    memset(&res, 0, sizeof(struct Response));
    if (recv(user->sockfd, &res, sizeof(struct Response), 0) < 0) {
        fprintf(stderr, "Error: receiving name message from user failed.\n");
        perror("recv");
        return -1;
    }
    len = strlen(res.client_message);
    res.client_message[len - 1] = '\0';
    sprintf(name, res.client_message);

    /*
     * Get passowrd from client
     */
    memset(&req, 0, sizeof(struct Request));
    sprintf(req.request, "password: ");
    if (send(user->sockfd, &req, sizeof(req), 0) < 0) {
        fprintf(stderr, "Error: sending password message to user failed.\n");
        perror("send");
        return -1;
    }

    memset(&res, 0, sizeof(struct Response));
    if (recv(user->sockfd, &res, sizeof(struct Response), 0) < 0) {
        fprintf(stderr, "Error: receiving name message from user failed.\n");
        perror("recv");
        return -1;
    }
    len = strlen(res.client_message);
    res.client_message[len - 1] = '\0';
    sprintf(password, res.client_message);

    /*
     * search user from databse
     */
    sprintf(query, "SELECT * FROM users WHERE name = '%s' AND password = '%s';", name, password);
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: fetching user info from db: %s.\n", mysql_error(user->db));
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(user->db);
    if (result == NULL) {
        fprintf(stderr, "Error: retrieving result set: %s.\n", mysql_error(user->db));
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row == NULL) {

        if (mysql_errno(user->db) != 0) {
            fprintf(stderr, "Error: fetching row: %s.\n", mysql_error(user->db));
            return -1;
        }

        memset(&res, 0, sizeof(struct Response));
        sprintf(res.server_message, "User doesn't exsit.\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending login user exist failed.\n");
            perror("send");
            return -1;
        }

        return 1;

    } else {

        /*
         * Update user name
         */
        memset(&res, 0, sizeof(struct Response));
        sprintf(user->name, name);

        /*
         * Update user status
         */
        user->status = USER;
        user->user_id = atoi(row[0]);


        /*
         * Sending sucessful login message
         */
        memset(&res, 0, sizeof(struct Response));
        sprintf(res.server_message, "Login successfully!\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending successful login message failed.\n");
            perror("send");
            return -1;
        }
    }
    
    return 0;
}

/*
 * Sign up a user and update database.
 * If a user name in database has existed, return 1.
 * return -1 for error, otherwise return 0.
 */
int signUp(struct User *user) {
    char            query[BUFSIZ];
    char            name[255];
    char            password[255];
    int             len;
    struct Response res;
    struct Request  req;
    memset(query, 0, sizeof(query));
    memset(name, 0, sizeof(name));
    memset(password, 0, sizeof(password));

    /*
     * Get name from client
     */
    memset(&req, 0, sizeof(struct Request));
    sprintf(req.request, "name: ");
    if (send(user->sockfd, &req, sizeof(req), 0) < 0) {
        fprintf(stderr, "Error: sending name message to user failed.\n");
        perror("send");
        return -1;
    }

    memset(&res, 0, sizeof(struct Response));
    if (recv(user->sockfd, &res, sizeof(struct Response), 0) < 0) {
        fprintf(stderr, "Error: receiving name message from user failed.\n");
        perror("recv");
        return -1;
    }
    len = strlen(res.client_message);
    res.client_message[len - 1] = '\0';
    sprintf(name, res.client_message);

    /*
     * Get passowrd from client
     */
    memset(&req, 0, sizeof(struct Request));
    sprintf(req.request, "password: ");
    if (send(user->sockfd, &req, sizeof(req), 0) < 0) {
        fprintf(stderr, "Error: sending password message to user failed.\n");
        perror("send");
        return -1;
    }

    memset(&res, 0, sizeof(struct Response));
    if (recv(user->sockfd, &res, sizeof(struct Response), 0) < 0) {
        fprintf(stderr, "Error: receiving name message from user failed\n");
        perror("recv");
        return -1;
    }
    len = strlen(res.client_message);
    res.client_message[len - 1] = '\0';
    sprintf(password, res.client_message);

    /*
     * search user from database
     */
    sprintf(query, "SELECT * FROM users WHERE name = '%s';", name);
    if (mysql_query(user->db, query)) {
        fprintf(stderr, "Error: fetching user info from db: %s.\n", mysql_error(user->db));
        return -1;
    }

    MYSQL_RES *result = mysql_store_result(user->db);
    if (result == NULL) {
        fprintf(stderr, "Error: retrieving result set: %s.\n", mysql_error(user->db));
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(result);
    if (row == NULL) {

        if (mysql_errno(user->db) != 0) {
            fprintf(stderr, "Error: fetching row: %s.\n", mysql_error(user->db));
            return -1;
        }

        /*
         * Update User table
         */
        memset(query, 0, sizeof(query));
        sprintf(query, "INSERT INTO users(name, password) VALUES('%s', '%s'); "
                       "SELECT LAST_INSERT_ID();", name, password);
        if (mysql_query(user->db, query)) {
            fprintf(stderr, "Error: updating user info to User table: %s.", mysql_error(user->db));
            return -1;
        }

        result = mysql_store_result(user->db);
        if (result == NULL) {
            fprintf(stderr, "Error: fetching result from users failed: %s.\n", mysql_error(user->db));
            return -1;
        }

        row = mysql_fetch_row(result);
        if (mysql_errno(user->db) != 0) {
            fprintf(stderr, "Error: fetching row from users failed: %s.\n", mysql_error(user->db));
            return -1;
        }

        /*
         * Update user name
         */
        sprintf(user->name, name);

        /*
         * Update user status
         */        
        user->status = USER;
        user->user_id = atoi(row[0]);

        /*
         * Sending sucessful login message
         */
        memset(&res, 0, sizeof(struct Response));
        sprintf(res.server_message, "Sign up successfully!\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending successful sign up message failed.\n");
            perror("send");
            return -1;
        }

    } else {

        memset(&res, 0, sizeof(struct Response));
        sprintf(res.server_message, "User has been registered.\n");
        res.method = SERVER_MESSAGE;
        if (send(user->sockfd, &res, sizeof(res), 0) < 0) {
            fprintf(stderr, "Error: sending sign up user name has been registered failed.\n");
            perror("send");
            return -1;
        }

        return 1;
    }

    return 0;
}
