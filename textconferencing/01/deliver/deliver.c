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

#define MAX_DATA 1000
#define MAX_NAME 12
#define PORT 8080
#define BUFFER_SIZE 1024


struct message
{                       
    unsigned int type; 
    unsigned int size; 
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

void textApp(int sockfd){
    char input_buffer[BUFFER_SIZE];
    printf("connection successful\n");
    printf("available commands:\n \\login <client ID> <password> <server-IP> <server-port>\n \\logout\n \\joinsession <session ID> \n \\leavesession \n \\createsession <session ID> \n \\list \n \\quit \n <text>\n");

    while(1){
        bzero(input_buffer, BUFFER_SIZE);

    }
}

int main(int argc, char *argv[])
{
    if(argc > 1){
        printf("usage: client\n");
        exit(0);
    }

    int sockfd, conn;
    struct sockaddr_in serv_addr, client;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        printf("socket creation error\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(PORT);

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("connection error\n");
        exit(0);
    }

    textApp(sockfd);

    close(sockfd);
    return 0;
}