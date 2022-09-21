#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[]){
    int sock;
    unsigned int address_size;
    unsigned short port;
    struct sockaddr_in server;
    char* ftp = "ftp";
    char user_input[32];
    int fileFound = -1;
    char dataBuffer[32];
    char *filename;

    if(argc != 3){
        printf("usage: deliver <server address> <server port number>\n");
        exit(0);
    }
    in_addr_t server_addr = inet_addr(argv[1]);
    port = htons(atoi(argv[2]));
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = port;
    server.sin_addr.s_addr = server_addr;


    printf("please input a message in the following format:\n");
    printf("ftp <file name>\n");

    fgets(user_input, 32, stdin);
    user_input[strlen(user_input)-1] = '\0';

    filename = strtok(user_input, " ");

    if(strcmp(filename, "ftp") == 0){
        filename = strtok(NULL, "\0");
    }else{
        printf("input needs to be: ftp <filename>\n");
        exit(1);
    }

    if(access(filename, F_OK) == 0){
        fileFound = 1;
    }else{
        fileFound = 0;
    }

    if(fileFound){
        sendto(sock, ftp, (strlen(ftp)+1), 0, (struct sockaddr *)&server, sizeof(server));
    }else{
        close(sock);
        return 0;
    }

    address_size = sizeof(server);

    recvfrom(sock, dataBuffer, sizeof(dataBuffer), 0, (struct sockaddr *) &server, &address_size);
    printf("%s",dataBuffer);
    close(sock);
    return 0;
}