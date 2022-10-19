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

#define BUFFER_SIZE 32
#define RECV_PACKET_SIZE 1100
#define MAX_PACKET_SIZE 1000
#define MIN(a, b) (a > b ? b : a)



double uniform_rand(){
    return (double) rand() / (double)((unsigned) RAND_MAX+1);
}

int main(int argc, char const *argv[]){
    
    if(argc != 2){
        printf("usage: server <udp listen port>\n");
        exit(0);
    }

    srand(time(NULL));

    char *ip = "127.0.0.1";
    int port = atoi(argv[1]);

    int sock;
    struct sockaddr_in server, client;
    char dataBuffer[RECV_PACKET_SIZE];
    socklen_t addr_size;

    char *ftp = "ftp";
    char *fileSent = "The file has been read and a binary data has been created";
    char *yes = "yes";
    char *ok = "OK";
    char *done = "DONE";
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    bind(sock, (struct sockaddr *)&server, sizeof(server));

    printf("server started on %d\n", port);

    addr_size = sizeof(client);

    char **packetsStrings;
    bool packetsAllocated = false;
    unsigned int receivedPackets = 0;
    unsigned int fileSize = 0;
    while (1)
    {
        int received = recvfrom(sock, dataBuffer, sizeof(dataBuffer), 0, (struct sockaddr *)&client, &addr_size);
        

        if (strcmp(dataBuffer, ftp) == 0)
        {
            printf("received ftp\n");
            sendto(sock, yes, (strlen(yes) + 1), 0, (struct sockaddr *)&client, sizeof(client));
            printf("sent yes\n");
        }
        else
        {   
            if(uniform_rand() > 1e-2){
                //printf("buffer: %s\n",dataBuffer);

                // create empty array of packets if not done yet
                unsigned int totalFrag = atoi(strtok(dataBuffer, ":"));
                if (!packetsAllocated && strcmp(dataBuffer, ftp) != 0)
                {   
                    //printf("%s\n", dataBuffer);
                    //printf("%d\n", totalFrag);
                    packetsStrings = malloc(sizeof(char *) * totalFrag);
                    packetsAllocated = true;
                }

                // ** FOR SOME
                unsigned int fragNo = atoi(strtok(NULL, ":"));
                printf("%d packets received\n", fragNo);
                unsigned int size = atoi(strtok(NULL, ":"));
                char *fileName = strtok(NULL, ":");
                char *filedata = fileName + strlen(fileName) + 1;
                // printf("fragno: %d\n",fragNo);
                // printf("size: %d\n",size);
                // printf("filename: %s\n",fileName);
                // printf("filedata: %ld\n", received - (int) (filedata - dataBuffer));

                packetsStrings[fragNo - 1] = malloc(sizeof(char) * size);
                memcpy(packetsStrings[fragNo - 1], filedata, sizeof(char) * size);
                receivedPackets++;
                fileSize += size;
                if (receivedPackets == totalFrag)
                {   
                    printf("all packets received\n");
                    FILE *fp = fopen(fileName, "wb");
                    //printf("total frag: %d\n",totalFrag);
                    for (unsigned int i = 0; i < totalFrag; i++)
                    {
                        //printf("frag: %d\n",i);
                        unsigned int wrote = fwrite(packetsStrings[i], 1, MIN(fileSize, MAX_PACKET_SIZE), fp);
                        fileSize -= wrote;
                        free(packetsStrings[i]);
                        //printf("freed\n");
                    }
                    
                    fclose(fp);

                    printf("%s created\n", fileName);
                    receivedPackets = 0;
                    fileSize = 0;
                    packetsAllocated = false;
                    free(packetsStrings);
                    packetsStrings = NULL;
                    sendto(sock, done, (strlen(done)+1), 0, (struct sockaddr *)&client, sizeof(client));
                }else{
                    sendto(sock, ok, (strlen(ok)+1), 0, (struct sockaddr *)&client, sizeof(client));
                    //sendto(sock, fileSent, (strlen(fileSent) + 1), 0, (struct sockaddr *)&client, sizeof(client));
                    //printf("sent file\n");
                }
            }else{
                printf("packet dropped\n");
            }
        }
    }

    close(sock);
    return 0;
}