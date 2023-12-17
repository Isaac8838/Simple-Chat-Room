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
    struct Response response;
    memset(query, 0, sizeof(query));    
    memset(name, 0, sizeof(name));
    memset(password, 0, sizeof(password));

    /*
     * Get name from client
     */
    memset(response.server_message, 0, sizeof(response.server_message));
    sprintf(response.server_message, "name: ");
    response.method = LOGIN_OR_SIGNUP;
    if (send(user->sockfd, &response, sizeof(response), 0) < 0) {
        fprintf(stderr, "Error: sending name message to user failed.\n");
        perror("send");
        return -1;
    }

    memset(response.client_message, 0, sizeof(response.client_message));
    if ((len = recv(user->sockfd, &response, sizeof(response), 0)) < 0) {
        fprintf(stderr, "Error: receiving name message from user failed.\n");
        perror("recv");
        return -1;
    }
    sprintf(name, response.client_message);

    /*
     * Get passowrd from client
     */
    memset(response.server_message, 0, sizeof(response.server_message));
    sprintf(response.server_message, "password: ");
    response.method = LOGIN_OR_SIGNUP;
    if (send(user->sockfd, &response, sizeof(response), 0) < 0) {
        fprintf(stderr, "Error: sending password message to user failed.\n");
        perror("send");
        return -1;
    }

    memset(response.client_message, 0, sizeof(response.client_message));
    if ((len = recv(user->sockfd, &response, sizeof(response), 0)) < 0) {
        fprintf(stderr, "Error: receiving name message from user failed.\n");
        perror("recv");
        return -1;
    }
    sprintf(password, response.client_message);

    /*
     * search user from databse
     */
    sprintf(query, "SELECT * FROM User WHERE name = '%s' AND password = '%s';", name, password);
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

        if (mysql_error(user->db)) {
            fprintf(stderr, "Error: fetching row: %s.\n", mysql_error(user->db));
            return -1;
        }

        memset(response.server_message, 0, sizeof(response.server_message));
        sprintf(response.server_message, "User doesn't exsit.\n");
        response.method = SERVER_MESSAGE;
        if (send(user->sockfd, &response, sizeof(response), 0) < 0) {
            fprintf(stderr, "Error: sending login user exist failed.\n");
            perror("send");
            return -1;
        }

        return 1;

    } else {

        /*
         * Update user name
         */
        memset(user->name, 0, sizeof(user->name));
        sprintf(user->name, name);

        /*
         * Update user status
         */
        user->status = USER;

        /*
         * Sending sucessful login message
         */
        memset(response.server_message, 0, sizeof(response.server_message));
        sprintf(response.server_message, "Login successfully!\n");
        response.method = SERVER_MESSAGE;
        if (send(user->sockfd, &response, sizeof(response), 0) < 0) {
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
    struct Response response;
    memset(query, 0, sizeof(query));
    memset(name, 0, sizeof(name));
    memset(password, 0, sizeof(password));

    /*
     * Get name from client
     */
    memset(response.server_message, 0, sizeof(response.server_message));
    sprintf(response.server_message, "name: ");
    response.method = LOGIN_OR_SIGNUP;
    if (send(user->sockfd, &response, sizeof(response), 0) < 0) {
        fprintf(stderr, "Error: sending name message to user failed.\n");
        perror("send");
        return -1;
    }

    memset(response.client_message, 0, sizeof(response.client_message));
    if ((len = recv(user->sockfd, &response, sizeof(response), 0)) < 0) {
        fprintf(stderr, "Error: receiving name message from user failed.\n");
        perror("recv");
        return -1;
    }
    sprintf(name, response.client_message);

    /*
     * Get passowrd from client
     */
    memset(response.server_message, 0, sizeof(response.server_message));
    sprintf(response.server_message, "password: ");
    response.method = LOGIN_OR_SIGNUP;
    if (send(user->sockfd, &response, sizeof(response), 0) < 0) {
        fprintf(stderr, "Error: sending password message to user failed.\n");
        perror("send");
        return -1;
    }

    memset(response.client_message, 0, sizeof(response.client_message));
    if ((len = recv(user->sockfd, &response, sizeof(response), 0)) < 0) {
        fprintf(stderr, "Error: receiving name message from user failed\n");
        perror("recv");
        return -1;
    }
    sprintf(password, response.client_message);

    /*
     * search user from database
     */
    sprintf(query, "SELECT * FROM User WHERE name = '%s';", name);
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

        if (mysql_error(user->db)) {
            fprintf(stderr, "Error: fetching row: %s.\n", mysql_error(user->db));
            return -1;
        }

        /*
         * Update User table
         */
        memset(query, 0, sizeof(query));
        sprintf(query, "INSERT INTO User(name, password, id) VALUES('%s', '%s', %d);", name, password, user->id);
        if (mysql_query(user->db, query)) {
            fprintf(stderr, "Error: updating user info to User table: %s.", mysql_error(user->db));
            return -1;
        }

        /*
         * Update user name
         */
        memset(user->name, 0, sizeof(user->name));
        sprintf(user->name, name);

        /*
         * Update user status
         */        
        user->status = USER;

        /*
         * Sending sucessful login message
         */
        memset(response.server_message, 0, sizeof(response.server_message));
        sprintf(response.server_message, "Sign up successfully!\n");
        response.method = SERVER_MESSAGE;
        if (send(user->sockfd, &response, sizeof(response), 0) < 0) {
            fprintf(stderr, "Error: sending successful sign up message failed.\n");
            perror("send");
            return -1;
        }

    } else {

        memset(response.server_message, 0, sizeof(response.server_message));
        sprintf(response.server_message, "%s has been registered.\n", name);
        response.method = SERVER_MESSAGE;
        if (send(user->sockfd, &response, sizeof(response), 0) < 0) {
            fprintf(stderr, "Error: sending sign up user name has been registered failed.\n");
            perror("send");
            return -1;
        }

        return 1;
    }

    return 0;
}
