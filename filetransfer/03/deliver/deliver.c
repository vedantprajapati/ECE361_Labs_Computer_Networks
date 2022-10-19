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

// packet format: "total_frag:frag_no:size:filename:filedata"
struct packet
{                            // packet format: "total_frag:frag_no:size:filename:filedata"
    unsigned int total_frag; // total number of fragments of the file
    unsigned int frag_no;    // sequence number of fragment
    unsigned int size;       // size of data, range [0,1000]
    char *filename;
    char filedata[MAX_PACKET_SIZE];
};

int fragment(FILE *file_to_send, char *buf)
{
    return fread(buf, sizeof(char), MAX_PACKET_SIZE, file_to_send);
}

unsigned int findSize(char *filename)
{
    FILE *fp = fopen(filename, "r");
    fseek(fp, 0L, SEEK_END);
    unsigned int res = ftell(fp);
    fclose(fp);
    return res;
}

int main(int argc, char *argv[])
{
    int sock;
    unsigned int address_size;
    unsigned short port;
    struct sockaddr_in server;
    char *ftp = "ftp";
    char user_input[32];
    int fileFound = 0;
    char dataBuffer[32];
    char *filename;
    char *binaryString;

    char *ok = "OK";
    char *done = "DONE";

    double rtt;
    bool timeout = false;
    int attempts = 0;

    clock_t begin, end;
    struct timeval ack_timeout;
    ack_timeout.tv_sec = 3;
    ack_timeout.tv_usec = 0;

    FILE* file_to_send;
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

    user_input[strlen(user_input) - 1] = '\0';

    filename = strtok(user_input, " ");

    if (strcmp(filename, "ftp") == 0)
    {
        filename = strtok(NULL, "\0");
    }
    else
    {
        printf("input needs to be: ftp <filename>\n");
        exit(1);
    }

    if (access(filename, F_OK) == 0)
    {
        fileFound = 1;
    }

    if (fileFound)
    {
        file_to_send = fopen(filename, "r");
        sendto(sock, ftp, (strlen(ftp) + 1), 0, (struct sockaddr *)&server, sizeof(server));
    }
    else
    {
        printf("file does not exist\n");
        close(sock);
        return 0;
    }

    address_size = sizeof(server);
    setsockopt(sock,SOL_SOCKET, SO_RCVTIMEO, (char*)&ack_timeout, sizeof(ack_timeout));

    if(recvfrom(sock, dataBuffer, sizeof(dataBuffer), 0, (struct sockaddr *)&server, &address_size) == -1){
        printf("no response from sever\n");
        close(sock);
        return 0;
    }
    

    if (strcmp(dataBuffer, "yes") == 0)
    {
        printf("A file transfer can start\n");
    }
    else
    {
        printf("received no\n");
        close(sock);
        return 0;
    }

    unsigned int fileSize = findSize(filename);

    // get the total number of fragments from the file size + max_packet size -1
    unsigned int totalFrag = (fileSize + MAX_PACKET_SIZE - 1) / MAX_PACKET_SIZE;

    char **fragments = malloc(sizeof(char *) * totalFrag);

    unsigned int j;
    for (j = 0; j < totalFrag; j++)
    {
        fragments[j] = malloc(sizeof(char) * MAX_PACKET_SIZE);
        fragment(file_to_send, fragments[j]);
    }

    char **packets = malloc(sizeof(char *) * totalFrag);
    int *packetSizes = malloc(sizeof(int) * totalFrag);

    unsigned int n;
    for (n = 1; n < totalFrag + 1; n++)
    {
        struct packet curPacket;

        curPacket.filename = filename;
        curPacket.total_frag = totalFrag;
        curPacket.frag_no = n;
        if (n != totalFrag)
        {
            curPacket.size = MAX_PACKET_SIZE;
        }
        else
        {
            curPacket.size = fileSize % MAX_PACKET_SIZE;
        }

        memset(curPacket.filedata, 0, sizeof(char) * MAX_PACKET_SIZE);

        packets[n - 1] = malloc(MAX_PACKET_SIZE * sizeof(char));
        memset(packets[n - 1], 0, MAX_PACKET_SIZE);
        unsigned int written = sprintf(packets[n - 1], "%d:%d:%d:%s:", curPacket.total_frag, curPacket.frag_no, curPacket.size, curPacket.filename);
        unsigned int packetSize = written + curPacket.size;
        packets[n - 1] = realloc(packets[n - 1], packetSize);
        memcpy(packets[n - 1] + written, fragments[n - 1], curPacket.size);
        packetSizes[n - 1] = packetSize;

        // fprintf(stderr, "%s\n", packets[n-1]);
    }

    struct packet recvPacket;

    memset(recvPacket.filedata,0,sizeof(char)*MAX_PACKET_SIZE);
    // recvPacket.filedata = malloc(sizeof(char)*MAX_PACKET_SIZE);
    char ackBuffer[10];

    
    for (n = 0; n < totalFrag; n++)
    {
        begin = clock();

        if(timeout){
            printf("retransmitting %d packet\n",n+1);
        }

        sendto(sock, packets[n], packetSizes[n], 0, (struct sockaddr *)&server, sizeof(server));
        
        if(recvfrom(sock, ackBuffer, sizeof(ackBuffer), 0, (struct sockaddr *) &server, &address_size) == -1){
            if(attempts < MAX_ATTEMPTS){
                timeout = true;
                n--;
                attempts++;
            }else{
                printf("max attempts reached\n");
                exit(3);
            }
            
        }else{
            timeout = false;
            attempts = 0;
            if(strcmp(ackBuffer, "OK") == 0){
                printf("%d/%d packets sent successfully\n", n+1, totalFrag);
                end = clock();
                rtt = (double) (end - begin) / CLOCKS_PER_SEC;
                printf("round trip time: %f seconds\n", rtt);

                if(n == 0){
                    ack_timeout.tv_sec = (int)(rtt < 0 ? (rtt - 0.5) : (rtt + 0.5));
                    ack_timeout.tv_usec = (int)((rtt - ack_timeout.tv_sec)*1000000);
                }
            }else if(strcmp(ackBuffer, "DONE") == 0){
                printf("%d/%d packets sent successfully\n", n+1, totalFrag);
                printf("finished sending packets\n");
            }else{
                printf("acknowledgement error\n");
                close(sock);
                return 0;
            }
        }
    }

    close(sock);
    return 0;
}