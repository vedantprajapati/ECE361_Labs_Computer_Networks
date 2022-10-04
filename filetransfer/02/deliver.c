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

#define MAX_PACKET_SIZE 1000
#define RECV_PACKET_SIZE 1100

struct packet { //packet format: "total_frag:frag_no:size:filename:filedata"
    unsigned int total_frag; //total number of fragments of the file
    unsigned int frag_no; //sequence number of fragment
    unsigned int size; //size of data, range [0,1000]
    char* filename;
    char filedata[MAX_PACKET_SIZE];
};

int fragment(FILE* file_to_send, char* buf){
    return fread(buf, sizeof(char), MAX_PACKET_SIZE, file_to_send);
}

int findSize(char* filename){
    FILE* fp = fopen(filename, "r");   
    fseek(fp, 0L, SEEK_END);
    int res = ftell(fp);
    fclose(fp);
    return res;
}

// int addIntToStr(char* inputStr, int num){
//     int size = (int)((ceil(log10(num))+1)*sizeof(char));
//     char str[size];
//     sprintf(str, "%d", num);
//     strcat(inputStr, str);
//     return 0;
// }

// char* formStartOfString(struct packet *packet, char *packetString){

    
//     // addIntToStr(packetString, packet->total_frag);
//     // strcat(packetString,":");
//     // addIntToStr(packetString, packet->frag_no);
//     // strcat(packetString,":");
//     // addIntToStr(packetString,packet->size);
//     // strcat(packetString,":");
//     // strcat(packetString,packet->filename);
//     // strcat(packetString,":");
//     //printf("%s\n",packetString);
    
//     return packetString;
// }

// int appendFragment(char* fragment, char* packetString, int size){
//     printf("total_frag : %s\n", strtok(fragment, ":"));
//     printf("frag_no: %s\n", strtok(fragment, ":"));
//     printf("size: %s\n", strtok(fragment, ":"));
//     printf("filename %s\n", strtok(fragment, ":"));
//     printf("filedata %s\n", strtok(fragment, ":"));

//     // int endOfString = strlen(packetString);
//     // memcpy(packetString+endOfString, fragment, sizeof(char)*size);
//     return 0;
// }

int stringToPacket(char* recvBuffer, struct packet *recvPacket){
    //packet format: "total_frag:frag_no:size:filename:filedata"
    char* token;
    token = strtok(recvBuffer,":");
    int count = 0;
    while(token != NULL){
        switch (count)
        {
        case 0:
            recvPacket->total_frag = token;
            count++;
            break;
        case 1:
            recvPacket->frag_no = token;
            count++;
            break;
        case 2:
            recvPacket->size = token;
            count++;
            break;
        case 3:
            recvPacket->filename = token;
            count++;
            break;
        case 4:
            recvPacket->filedata = token;
            count++;
            break;
        default:
            printf("string to packet error\n");
            exit(2);
            break;
        }
    }
    return 0;
}

int main(int argc, char *argv[]){
    int sock;
    unsigned int address_size;
    unsigned short port;
    struct sockaddr_in server;
    char* ftp = "ftp";
    char user_input[32];
    int fileFound = 0;
    char dataBuffer[32];
    char *filename;
    char* binaryString; 

    time_t begin, end;

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
    }

    if(fileFound){
        file_to_send = fopen(filename, "r");
        time(&begin);
        sendto(sock, ftp, (strlen(ftp)+1), 0, (struct sockaddr *)&server, sizeof(server));
    }else{
        printf("file does not exist\n");
        close(sock);
        return 0;
    }

    address_size = sizeof(server);

    recvfrom(sock, dataBuffer, sizeof(dataBuffer), 0, (struct sockaddr *) &server, &address_size);
    time(&end);
    printf("round trip time: %f secondss\n", difftime(end, begin));

    if(strcmp(dataBuffer, "yes") == 0){
        printf("A file transfer can start\n");
    }else{
        printf("received no\n");
        close(sock);
        return 0;
    }
    
    int fileSize = findSize(filename);
    int totalFrag = (fileSize + MAX_PACKET_SIZE - 1) / MAX_PACKET_SIZE;   

    char **fragments = malloc(sizeof(char*)*totalFrag);
    
    int j;
    for(j = 0; j < totalFrag; j++){
        fragments[j] = malloc(sizeof(char)*MAX_PACKET_SIZE);
        fragment(file_to_send, fragments[j]);
    }

    char **packets = malloc(sizeof(char*) * totalFrag);

    int n;
    for(n = 1; n < totalFrag+1; n++){
        struct packet curPacket;

        curPacket.filename = filename;
        curPacket.total_frag = totalFrag;
        curPacket.frag_no = n;
        if(n != totalFrag){
            curPacket.size = MAX_PACKET_SIZE;
        }else{
            curPacket.size = fileSize % MAX_PACKET_SIZE;
        }
        //printf("%s", curPacket.filename);
        //printf("%d", curPacket.frag_no );
        //printf("%d", totalFrag );


        memset(curPacket.filedata, 0, sizeof(char)*MAX_PACKET_SIZE);
        
        packets[n - 1] = malloc(MAX_PACKET_SIZE*sizeof(char));
        memset(packets[n - 1], 0, MAX_PACKET_SIZE);
        int written = sprintf(packets[n - 1], "%d:%d:%d:%s:", curPacket.total_frag, curPacket.frag_no, curPacket.size, curPacket.filename);
        int packetSize = written + curPacket.size;
        packets[n - 1] = realloc(packets[n - 1], packetSize);
        memcpy(packets[n - 1] + written, fragments[n - 1], curPacket.size);

        fprintf(stderr, "%s\n", packets[n-1]);
    }

    struct packet recvPacket;
    recvPacket.filedata = malloc(sizeof(char)*MAX_PACKET_SIZE);
    
    char *recvBuffer[RECV_PACKET_SIZE];


    for(n = 0; n < totalFrag; n++){
        sendto(sock, packets[n], (strlen(packets[n])+1), 0, (struct sockaddr *)&server, sizeof(server));
        memset(recvBuffer,0, sizeof(char)*RECV_PACKET_SIZE);
        recvfrom(sock, recvBuffer, sizeof(recvBuffer), 0, (struct sockaddr *) &server, &address_size);

        stringToPacket(recvBuffer, &recvPacket);
    }
    
    close(sock);
    return 0;
}