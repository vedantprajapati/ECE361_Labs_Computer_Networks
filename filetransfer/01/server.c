#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 32

int main(int argc, char const *argv[]){

    if(argc != 2){
        printf("usage: server <udp listen port>\n");
        exit(0);
    }

    char *ip = "127.0.0.1";
    int port = atoi(argv[1]);

    int sock;
    struct sockaddr_in server, client;
    char dataBuffer[BUFFER_SIZE];
    socklen_t addr_size;

    char *ftp = "ftp";
    char *no = "no";
    char *yes = "yes";

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if (bind(sock, (struct sockaddr *)&server, sizeof(server)) == -1){
        printf("Error: Cannot bind socket to server\n");
        exit(1);
    }

    printf("server started on %d\n", port);

    addr_size = sizeof(client);

    while(1){
        if (recvfrom(sock, dataBuffer, sizeof(dataBuffer), 0, (struct sockaddr *) &client, &addr_size) == -1){
            printf("Error: Cannot recieve information from socket\n");
            exit(1);
        };

        if(strcmp(dataBuffer, ftp) == 0){
            printf("received ftp\n");
            if (sendto(sock, yes, (strlen(yes)+1), 0, (struct sockaddr *)&client, sizeof(client)) == -1){
                printf("Error: Cannot recieve information from socket\n");
                exit(1);
            };
            printf("sent yes\n");
        }else{
            if (sendto(sock, no, (strlen(no)+1), 0, (struct sockaddr *)&client, sizeof(client)) == -1){
                printf("Error: Cannot recieve information from socket\n");
                exit(1);
            };
            printf("sent no\n");
        }
    }

    close(sock);
    return 0;
}