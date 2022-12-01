#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include "../helpers.h"

#define MAX_DATA 1000
#define MAX_NAME 12
#define BUFFER_SIZE 1024
#define BACKLOG 3
// #define NUM_OF_USERS 10
struct users *users;
struct sessions *sessions;
int session_count = 0;
char buffer[BUFFER_SIZE];

#define SA struct sockaddr

void exit_func(struct message *recvd_packet)
{
    printf("Exiting instance of server\n");
    char *username = strtok((char *)recvd_packet->source, " ");
    rm_user_id(users, username);
    printf("User %s has been removed from the list of logged in users\n", username);
}

void join(struct message *recvd_packet)
{

    int i = 0;
    char *curr_username = recvd_packet->source;
    char *curr_session = recvd_packet->data;
    struct user *user = lookup_user_name(users, curr_username);
    struct session *user_session = lookup_session(sessions, user->session_id);
    struct session *session = lookup_session(sessions, curr_session);

    // if user isnt in a session
    if (user_session != NULL)
    {
        printf("User %s is already in a session\n", curr_username);
        return;
    }
    else if (session == NULL)
    {
        printf("Session %s does not exist\n", curr_session);
        return;
    }
    else
    {
        printf("User %s is joining session %s\n", curr_username, curr_session);
        bool user_joined = add_user_to_session(users, sessions, session, user);
        if (user_joined)
        {
            printf("User %s has joined session %s\n", curr_username, curr_session);
        }
        else
        {
            printf("User %s could not join session %s\n", curr_username, curr_session);
        }
        return;
    }
}

void leave_session(struct message *recvd_packet)
{

    char *curr_username = recvd_packet->source;
    char *curr_session = recvd_packet->data;
    struct session *session = lookup_session(sessions, session->id);
    struct user *user = lookup_user_name(users, curr_username);
    bool user_removed = rm_user_from_session(sessions, users, session, user);

    if (user_removed)
    {
        printf("User %s has left session %s\n", curr_username, curr_session);
    }
    else
    {
        printf("User %s could not leave session %s\n", curr_username, curr_session);
    }
    return;
}

void new_session(struct message *recvd_packet)
{

    char *curr_username = recvd_packet->source;
    char *curr_session = recvd_packet->data;

    struct session *session = lookup_session(sessions, curr_session);
    if (session != NULL)
    {
        printf("Session %s already exists\n", curr_session);
        return;
    }
    else
    {

        bool session_created = add_session(sessions, curr_session);
        if (session_created)
        {
            printf("Session %s has been created\n", curr_session);
        }
        else
        {
            printf("Session %s could not be created\n", curr_session);
        }
        return;
    }
    session_count++;
}

void send_message(int connfd, struct message* recvd_packet)
{
    char *curr_username = recvd_packet->source;
    struct user *user = lookup_user_name(users, curr_username);\
    char * text = recvd_packet->data;
    char * user_session = user->session_id;

    if(!user){
        text = "User not online";
        display_message(buffer, JN_NAK, strlen(text), "client", text);
    }
    else{
        display_message(buffer, JN_NAK, strlen(text), "client", text);
    }

    char * text = "Hello from server";
    display_message(buffer, LO_NAK, strlen(text), "client", text);
    write(connfd, buffer, strlen(buffer));
    while(users != NULL){
        if(strcmp(users->user->session_id, user_session) == 0){
            ssize_t sent = write(users->user->connfd, buffer, strlen(buffer));
            if (sent == -1) {
                printf("Error sending message to user %s\n", users->user->username);
                exit(1);
            }
        }
        users = users->next;
    }
}

void query(int connfd)
{
    char text[BUFFER_SIZE];

    strcat(text, "The following users are online: ");
    while(users != NULL){
        strcat(text, users->user->username);
        strcat(text, ", ");
        users = users->next;
    }

    strcat(text, "\nThe following sessions are active: ");
    while(sessions != NULL){
        strcat(text, sessions->session->id);
        strcat(text, ", ");
        sessions = sessions->next;
    }
    display_message(buffer, QU_ACK, strlen(text), "client", text);
    if (write(connfd, buffer, strlen(buffer)) == -1) {
        printf("Error sending message to user %s\n", users->user->username);
        exit(1);
    }


}

void login(int connfd, struct message *recvd_packet)
{
    char *packet;
    // if user is active, then print already logged in
    char* username = strtok((char*)recvd_packet->data, " ");
    char* password = strtok(NULL, " ");
    struct user *user = lookup_user_creds(recvd_packet->source, "login.txt");

    if (user != NULL && user->sock_fd)
    {
        printf("user already logged in\n");
        packet = "2:0:server:user already logged in";

        exit(0);
    }
    // if user is inactive and password is correct, then set socket fd
    else if (user != NULL && !user->sock_fd)
    {
        user->sock_fd = connfd;
        packet = "1:0:server:ok";
    }
    // if user is inactive and password is incorrect, then print incorrect password/username
    else
    {
        printf("invalid username or password\n");
        printf("username: %s\n", recvd_packet->source);
        packet = "2:0:server:incorrect username or password";
        exit(0);
    }
    printf("%s\n", packet);
    write(connfd, packet, sizeof(packet));
    return;
}

void textApp(int connfd, int sockfd, struct sockaddr_in cli_addr, socklen_t len)
{
    char input_buffer[BUFFER_SIZE];
    struct message recvd_packet;

    while (1)
    {
        bzero(input_buffer, BUFFER_SIZE);

        if (read(connfd, input_buffer, BUFFER_SIZE) < 0)
        {
            printf("read error\n");
            exit(0);
        }
        printf("%s\n", input_buffer);
        convert_client_input_to_packet(input_buffer, &recvd_packet);
        display_packet(&recvd_packet);


        switch (recvd_packet.type)
        {
        case LOGIN:
            login(connfd, &recvd_packet);
            break;
        case EXIT: // exit
            exit_func(&recvd_packet);
            break;
        case JOIN: // join
            join(&recvd_packet);
            break;
        case LEAVE_SESS: // leave_session
            leave_session(&recvd_packet);
            break;
        case NEW_SESS: // new_session
            new_session(&recvd_packet);
            break;
        case MESSAGE: // message
            send_message();
            break;
        case QUERY: // query
            query();
            break;
        default:
            printf("invalid type \n");
        }

        connfd = accept(sockfd, (struct sockaddr *)&cli_addr, &len);
    }
    close(connfd);
}

int main(int argc, char const *argv[])
{
    if (argc != 2)
    {
        printf("usage: server <TCP port to listen on>\n");
        exit(0);
    }

    int port = atoi(argv[1]);

    int sockfd, connfd;
    struct sockaddr_in serv_addr, cli_addr;
    socklen_t len;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("socket creation error\n");
        exit(0);
    }
    else
    {
        printf("socket creation successful\n");
    }
    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // constant used to store any IP address assigned to server
    serv_addr.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("bind error\n");
        exit(0);
    }
    else
    {
        printf("binding successful\n");
    }

    if (listen(sockfd, BACKLOG) != 0)
    {
        printf("listen error\n");
        exit(0);
    }
    else
    {
        printf("listen successful\n");
    }

    len = sizeof(cli_addr);
    connfd = accept(sockfd, (struct sockaddr *)&cli_addr, &len);

    if (connfd < 0)
    {
        printf("accept error\n");
        exit(0);
    }
    else
    {
        printf("accepted successfully\n");
    }
    textApp(connfd, sockfd, cli_addr, len);

    close(sockfd);
    return 0;
}