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
#include <math.h>
#include <stdbool.h>
#include <errno.h>

#define MAX_DATA 1000
#define MAX_NAME 12
#define MAX_PASSWORD 16
#define BUFFER_SIZE 1024
#define PORT 8080

#define SA struct sockaddr

void textApp(int sockfd);

struct message
{
    unsigned int type;
    unsigned int size;
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

typedef struct message Message;

void process_input(char *input_buffer, Message *packet)
{
    // printf(input_buffer);
    char *token = strtok(input_buffer, ":");
    int i = 0;
    while (token != NULL)
    {
        switch (i)
        {
        case 0:
            packet->type = atoi(token);
            token = strtok(NULL, ":");
            i++;
            break;
        case 1:
            packet->size = atoi(token);
            token = strtok(NULL, ":");
            i++;
            break;
        case 2:
            strcpy(packet->source, token);
            token = strtok(NULL, ":");
            i++;
            break;
        case 3:
            strcpy(packet->data, token);
            token = strtok(NULL, ":");
            i++;
            break;
        default:
            printf("invalid packet - too many arguements\n");
            exit(0);
        }
    }
}

void login()
{
    char input_buffer[BUFFER_SIZE];
    char recv_buff[BUFFER_SIZE];
    bool invalid_command = true;
    char *client_id = NULL;
    char *password = NULL;
    char *ip = NULL;
    char *port = NULL;

    while (invalid_command)
    {
        printf("available commands:\n \\login <client ID> <password> <server-IP> <server-port>\n");
        fgets(input_buffer, BUFFER_SIZE, stdin);

        char *token = strtok(input_buffer, " ");
        if (strcmp(token, "\\login") == 0)
        {
            client_id = strtok(NULL, " ");
            password = strtok(NULL, " ");
            ip = strtok(NULL, " ");
            port = strtok(NULL, " ");
            invalid_command = false;
        }
        else
        {
            printf("invalid command\n");
        }
    }

    int sockfd;
    struct sockaddr_in serv_addr, client;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("socket creation error\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(atoi(port));

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("connection error\n");
        exit(0);
    }

    char *packet[MAX_DATA];
    strcpy(packet, "0:0:");
    strcat(packet, client_id);
    strcat(packet, ":");
    strcat(packet, password);
    // printf("%s",packet);

    write(sockfd, packet, sizeof(packet));
    read(sockfd, recv_buff, BUFFER_SIZE);

    Message recvd_packet;
    process_input(recv_buff, &recvd_packet);
    printf("%s\n", recv_buff);

    if (recvd_packet.type == 1)
    {
        textApp(sockfd);
        close(sockfd);
    }
    else if (recvd_packet.type == 2)
    {
        printf("login failure: %s\n", recvd_packet.data);
        login();
    }
    else
    {
        printf("login error\n");
        exit(0);
    }
}

void textApp(int sockfd)
{
    char input_buffer[BUFFER_SIZE];
    printf("log in successful\n \n");

    printf("available commands:\n \\logout\n \\joinsession <session ID> \n \\leavesession \n \\createsession <session ID> \n \\list \n \\quit \n <text>\n");

    while (1)
    {
        bzero(input_buffer, BUFFER_SIZE);
    }
}

int main(int argc, char *argv[])
{
    if (argc > 1)
    {
        printf("usage: client\n");
        exit(0);
    }

    login();

    return 0;
}