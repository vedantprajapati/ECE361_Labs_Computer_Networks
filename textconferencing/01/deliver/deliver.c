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
#define MAX_PASSWORD 16
#define BUFFER_SIZE 1024


struct message
{                       
    unsigned int type; 
    unsigned int size; 
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

typedef struct message Message;

void process_input(char* input_buffer, Message* packet){
    char *token = strtok(input_buffer, " ");
    int i = 0;
    while(token != NULL){
        switch(i){
            case 0:
                packet->type = token;
                i++;
                token = strtok(NULL, input_buffer);
                break;
            case 1:
                packet->size = atoi(token);
                i++;
                token = strtok(NULL, input_buffer);
                break;
            case 2:
                strlcpy(packet->source,token,MAX_NAME);
                i++;
                token = strtok(NULL, input_buffer);
                break;
            case 3:
                strlcpy(packet->data,token,MAX_DATA);
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
}

void login(){
    char input_buffer[BUFFER_SIZE];
    bool invalid_login = true;
    char* client_id, password, ip, port;

    while(true){
        printf("available commands:\n \\login <client ID> <password> <server-IP> <server-port>\n");
        fgets(input_buffer, BUFFER_SIZE, stdin);

        char* token = strtok(input_buffer, " ");
        if(strcmp(token, "\\login")==0){
            int i = 0;
            while(token != NULL){
                if(i == 0){
                    strlcpy(client_id, token, strlen(token));
                }else if(i == 1){
                    strlcpy(password, token, strlen(token));
                }else if(i == 2){
                    strlcpy(ip, token, strlen(token));
                }else if(i == 3){
                    strlcpy(port, token, strlen(token));
                }else{
                    printf("invalid command\n");
                    continue;
                }
                token = strtok(input_buffer, " ");
                i++;
            }
            if(i == 4){
                invalid_login = false;
            }else{
                printf("invalid command\n");
            }
        }else{
            printf("invalid command\n");
        }
    }
    int sockfd;
    struct sockaddr_in serv_addr, client;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0){
        printf("socket creation error\n");
        exit(0);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip);
    serv_addr.sin_port = htons(port);

    if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("connection error\n");
        exit(0);
    }



    textApp(sockfd);

    close(sockfd);
}

void textApp(int sockfd){
    char input_buffer[BUFFER_SIZE];
    printf("connection successful\n");

    login(sockfd);
    printf("available commands:\n \\login <client ID> <password> <server-IP> <server-port>\n\\logout\n \\joinsession <session ID> \n \\leavesession \n \\createsession <session ID> \n \\list \n \\quit \n <text>\n");

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

    login();

    return 0;
}