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

#define BUFFER_SIZE 32
#define RECV_PACKET_SIZE 1100
#define MAX_PACKET_SIZE 1000
#define MIN(a, b) (a > b ? b : a)

int main(int argc, char const *argv[])
{

    if (argc != 2)
    {
        printf("usage: server <udp listen port>\n");
        exit(0);
    }

    char *ip = "127.0.0.1";
    int port = atoi(argv[1]);

    int sock;
    struct sockaddr_in server, client;
    char dataBuffer[RECV_PACKET_SIZE];
    socklen_t addr_size;

    char *ftp = "ftp";
    char *fileSent = "The file has been read and a binary data has been created";
    char *yes = "yes";

    sock = socket(AF_INET, SOCK_DGRAM, 0);

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    bind(sock, (struct sockaddr *)&server, sizeof(server));

    printf("server started on %d\n", port);

    addr_size = sizeof(client);

    char **packetsStrings;
    bool packetsAllocated = false;
    int receivedPackets = 0;
    int fileSize = 0;
    while (1)
    {
        recvfrom(sock, dataBuffer, sizeof(dataBuffer), 0, (struct sockaddr *)&client, &addr_size);

        if (strcmp(dataBuffer, ftp) == 0)
        {
            printf("received ftp\n");
            sendto(sock, yes, (strlen(yes) + 1), 0, (struct sockaddr *)&client, sizeof(client));
            printf("sent yes\n");
        }
        else
        {
            // create empty array of packets if not done yet
            int totalFrag = atoi(strtok(dataBuffer, ":"));

            if (!packetsAllocated && strcmp(dataBuffer, ftp) != 0)
            {
                printf("%s\n", dataBuffer);

                printf("%d\n", totalFrag);
                packetsStrings = malloc(sizeof(char *) * totalFrag);
                packetsAllocated = true;
            }

            // ** FOR SOME
            int fragNo = atoi(strtok(NULL, ":"));

            printf("got %d\n", fragNo);
            int size = atoi(strtok(NULL, ":"));
            char *fileName = strtok(NULL, ":");
            char *filedata = fileName + strlen(fileName) + 1;

            packetsStrings[fragNo - 1] = malloc(sizeof(char) * size);
            memcpy(packetsStrings[fragNo - 1], filedata, sizeof(char) * size);

            receivedPackets++;
            fileSize += size;

            if (receivedPackets == totalFrag)
            {
                FILE *fp = fopen(fileName, "wb");
                for (int i = 0; i < totalFrag; i++)
                {
                    int wrote = fwrite(packetsStrings[i], 1, MIN(fileSize, MAX_PACKET_SIZE), fp);
                    fileSize -= wrote;
                    free(packetsStrings[i]);
                }
                fclose(fp);

                receivedPackets = 0;
                fileSize = 0;
                packetsAllocated = false;
                free(packetsStrings);
                packetsStrings = NULL;
                sendto(sock, fileSent, (strlen(fileSent) + 1), 0, (struct sockaddr *)&client, sizeof(client));
                printf("sent file\n");
            }
        }
    }

    close(sock);
    return 0;
}