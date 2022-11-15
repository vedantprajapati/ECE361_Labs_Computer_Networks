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
#include "user.h"

#define MAX_DATA 1000
#define MAX_NAME 12
#define BUFFER_SIZE 1024
#define BACKLOG 3
#define NUM_OF_USERS 2

#define SA struct sockaddr
// //packet format = type:size:source:data
struct message
{                       
    unsigned int type; 
    unsigned int size; 
    unsigned char source[MAX_NAME];
    unsigned char data[MAX_DATA];
};

typedef struct message Message;

struct user users[10] = {{.username="user1", .password="pass1", .active=false, .ip=NULL, .port=NULL, .session_id=NULL},
                         {.username="user2", .password="pass2", .active=false, .ip=NULL, .port=NULL, .session_id=NULL},
                         {.username="user3", .password="pass3", .active=false, .ip=NULL, .port=NULL, .session_id=NULL},
                         {.username="user4", .password="pass4", .active=false, .ip=NULL, .port=NULL, .session_id=NULL},
                         {.username="user5", .password="pass5", .active=false, .ip=NULL, .port=NULL, .session_id=NULL},
                         {.username="user6", .password="pass6", .active=false, .ip=NULL, .port=NULL, .session_id=NULL},
                         {.username="user7", .password="pass7", .active=false, .ip=NULL, .port=NULL, .session_id=NULL},
                         {.username="user8", .password="pass8", .active=false, .ip=NULL, .port=NULL, .session_id=NULL},
                         {.username="user9", .password="pass9", .active=false, .ip=NULL, .port=NULL, .session_id=NULL},
                         {.username="user10", .password="pass10", .active=false, .ip=NULL, .port=NULL, .session_id=NULL}};

void process_input(char* input_buffer, Message* packet){
    //printf("%s",input_buffer);
    char *token = strtok(input_buffer, ":");
    int i = 0;
    while(token != NULL){
        switch(i){
            case 0:
                packet->type = atoi(token);
                token = strtok(NULL,":");
                i++;
                break;
            case 1:
                packet->size = atoi(token);
                token = strtok(NULL,":");
                i++;
                break;
            case 2:
                strcpy(packet->source, token);
                token = strtok(NULL,":");
                i++;
                break;
            case 3:
                strcpy(packet->data, token);
                token = strtok(NULL,":");
                i++;
                break;
            default:
                printf("invalid packet - too many arguements\n");
                exit(0);
        }
    }
}

void exit_func(){

}

void join(){

}

void leave_session(){

}

void new_session(){

}

void message(){

}

void query(){

}

void login(int connfd){
    char input_buffer[BUFFER_SIZE];
    Message recvd_packet;

    read(connfd, input_buffer, BUFFER_SIZE);

    process_input(input_buffer, &recvd_packet);

    bool auth = false;
    bool logged_in = false;
    for(int i = 0; i < NUM_OF_USERS; i++){
        if(strcmp(users[i].username, recvd_packet.source) == 0 && strcmp(users[i].password,recvd_packet.data)==0){
            auth = true;
            if(!users[i].active){
                users[i].active = true;
                printf("match correct\n");
            }else{
                logged_in = true;
            }
        }
    }
    char *packet;
    if(auth && !logged_in){
        packet = "1:0:server:ok";
    }else if(!auth){
        printf("auth false");
        packet = "2:0:server:incorrect username or password";
    }else{
        printf("already logged");
        packet = "2:0:server:user already logged in";
    }

    write(connfd, packet, sizeof(packet));
}
                         
char **sessions;
int session_count = 0;

void textApp(int connfd){
    char input_buffer[BUFFER_SIZE];
    struct message recvd_packet;
    int conn_fd = connfd;
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
        char *curr_username;
        int h = 0;
        switch(recvd_packet.type){
            case 1://exit
                exit_func();
                break;
            case 2://join
                curr_username = recvd_packet.source;
                while(sessions[i]) {
                    if(strcmp(sessions[i], recvd_packet.data) == 0) {
                        //session already exists
                        for (int i = 0; i < 10; i++){
                            if (strcmp(users[i].username, curr_username) == 0 && users[i].active == true){
                                users[i].session_id = recvd_packet.data;
                                break;
                            }
                        }
                        printf("user %s has left session %s", curr_username, recvd_packet.data);
                    }
                    i++;
                }

            case 3://leave_session
                curr_username = recvd_packet.source;
                for (int i = 0; i < 10; i++){
                    if (strcmp(users[i].username, curr_username) == 0 && users[i].active == true){
                        users[i].session_id = NULL;
                        break;
                    }
                }
                printf("user %s has left session %s", curr_username, recvd_packet.data);
            case 4://new_session
                h = 0;
                char *session_id=recvd_packet.data;
                if(sessions == NULL){
                    sessions = malloc(session_count * sizeof(char*));
                }
                while(sessions[h]) {
                    if(strcmp(sessions[h], session_id) == 0) {
                        //session already exists
                        printf('session already exists');                        
                        exit(0);
                    }
                    h++;
                }
                sessions[h] = malloc((strlen(recvd_packet.data)+1) * sizeof(char)); // yeah, I know sizeof(char) is 1, but to make it clear...
                strcpy(sessions[h], recvd_packet.data);
                session_count++;
            case 5://message
                message();
                break;
            case 6://query
                query();
                break;
            default:
                printf("invalid type \n");
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
    }else{
        printf("socket creation successful\n");
    }
    bzero(&serv_addr, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); //constant used to store any IP address assigned to server
    serv_addr.sin_port = htons(port);

    if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        printf("bind error\n");
        exit(0);
    }else{
        printf("binding successful\n");
    }

    if(listen(sockfd, BACKLOG) != 0){
        printf("listen error\n");
        exit(0);
    }else{
        printf("listen successful\n");
    }

    len = sizeof(cli_addr);
    connfd = accept(sockfd, (struct sockaddr *)&cli_addr, &len);

    if(connfd < 0){
        printf("accept error\n");
        exit(0);
    }else{
        printf("accepted successfully\n");
    }

    login(connfd);

    close(sockfd);
    return 0;
}