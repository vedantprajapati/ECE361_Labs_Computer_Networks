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

#define MAX_PACKET_SIZE 1000
#define RECV_PACKET_SIZE 1100
#define MAX_ATTEMPTS 3
#define PORT 8080

// packet format: "total_frag:frag_no:size:filename:filedata"
struct packet
{                            // packet format: "total_frag:frag_no:size:filename:filedata"
    unsigned int total_frag; // total number of fragments of the file
    unsigned int frag_no;    // sequence number of fragment
    unsigned int size;       // size of data, range [0,1000]
    char *filename;
    char filedata[MAX_PACKET_SIZE];
};

void textApp(int sock){
    while(1){

    }
}

int main(int argc, char *argv[])
{
    int sock, conn;
    struct sockaddr_in server, client;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0){
        printf("socket creation error\n");
        exit(0);
    }

    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr("127.0.0.1");
    server.sin_port = htons(PORT);

    if(connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0){
        printf("connection error\n");
        exit(0);
    }

    textApp(sock);

    close(sock);
    return 0;
}