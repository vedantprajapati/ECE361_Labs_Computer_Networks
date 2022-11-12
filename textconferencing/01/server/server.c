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

void textApp(int conn){
    while(1){
    }
}

int main(int argc, char const *argv[])
{

    if (argc != 2)
    {
        printf("usage: server <TCP port to listen on>\n");
        exit(0);
    }

    int port = atoi(argv[1]);

    int sock, conn, len;
    struct sockaddr_in server, client;

    sock = socket(AF_INET, SOCK_STREAM, 0);

    if(sock < 0){
        printf("socket creation error\n");
        exit(0);
    }

    bzero(&server, sizeof(server));

    server.sin_family = AF_INET6;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(port);

    if(bind(sock, (struct sockaddr *)&server, sizeof(server)) < 0){
        printf("bind error\n");
        exit(0);
    };

    if(listen(sock, 3) != 0){
        printf("listen error\n");
        exit(0);
    }

    len = sizeof(client);
    conn = accept(sock, (struct sockaddr *)&client, &len);

    textApp(conn);

    close(sock);
    return 0;
}