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

int main(int argc, char const **argv[]){
    
    if(argc != 2){
        printf("usage: server <udp listen port>\n");
        exit(0);
    }

    char *ip = "127.0.0.1";
    int port = atoi(argv[1]);

    int sock, n;
    struct sockaddr_in server, client;
    char dataBuffer[BUFFER_SIZE];
    socklen_t addr_size;

    char *ftp = "ftp";
    char *no = "no";
    char *yes = "yes";
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    memset(&server, '\0', sizeof(server));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    bind(sock, (struct sockaddr *)&server, sizeof(server));

    addr_size = sizeof(client);

    recvfrom(sock, dataBuffer, sizeof(dataBuffer), 0, (struct sockaddr *) &client, &addr_size);

    if(strcmp(dataBuffer, ftp) == 0){
        sendto(sock, yes, (strlen(yes)+1), 0, (struct sockaddr *)&client, sizeof(client));
    }else{
        sendto(sock, no, (strlen(no)+1), 0, (struct sockaddr *)&client, sizeof(client));
    }


    
    return 0;
}