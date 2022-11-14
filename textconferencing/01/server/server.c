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

#define MAX_DATA 1000
#define MAX_NAME 12
#define BUFFER_SIZE 1024
#define BACKLOG 3
//packet format = type:size:source:data
struct message
{                       
    unsigned int type; 
    unsigned int size; 
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

void textApp(int connfd){
    char input_buffer[BUFFER_SIZE];
    struct message recvd_packet;
    while(1){
        bzero(input_buffer, BUFFER_SIZE);

        if(read(connfd, input_buffer, BUFFER_SIZE)<0){
            printf("read error\n");
            exit(0);
        }
        
        printf("%s",input_buffer);
        int i = 0;
        char *token = strtok(input_buffer, ":");

        while(token != NULL){
            switch(i){
                case 0:
                    recvd_packet.type = token;
                    i++;
                    token = strtok(NULL, input_buffer);
                    break;
                case 1:
                    recvd_packet.size = atoi(token);
                    i++;
                    token = strtok(NULL, input_buffer);
                    break;
                case 2:
                    strlcpy(recvd_packet.source,token,MAX_NAME);
                    i++;
                    token = strtok(NULL, input_buffer);
                    break;
                case 3:
                    strlcpy(recvd_packet.data,token,MAX_DATA);
                    token = strtok(NULL, input_buffer);
                    break;
                default:
                    printf("packet format error - too many segments\n");
                    exit(0);
            }
        }

        if(i != 3){
            printf("packet format error - too few segments\n");
            exit(0);
        }

        switch(recvd_packet.type){
            case 0://login
            case 1://exit
            case 2://join
            case 3://leave_session
            case 4://new_session
            case 5://message
            case 6://query
            default:
                printf("unknown type error\n");
                exit(0);
        }

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
    if(sockfd < 0){
        printf("socket creation error\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET6;
    serv_addr.sin_addr.s_addr = INADDR_ANY; //constant used to store any IP address assigned to server
    serv_addr.sin_port = htons(port);

    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("bind error\n");
        exit(0);
    };

    if(listen(sockfd, BACKLOG) != 0){
        printf("listen error\n");
        exit(0);
    }

    len = sizeof(cli_addr);
    connfd = accept(sockfd, (struct sockaddr *)&cli_addr, &len);

    if(connfd < 0){
        printf("accept error\n");
        exit(0);
    }

    textApp(connfd);

    close(sockfd);
    return 0;
}