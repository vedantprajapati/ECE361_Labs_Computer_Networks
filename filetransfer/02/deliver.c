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

struct packet { //packet format: "total_frag:frag_no:size:filename:filedata"
    unsigned int total_frag; //total number of fragments of the file
    unsigned int frag_no; //sequence number of fragment
    unsigned int size; //size of data, range [0,1000]
    char* filename;
    char filedata[1000];
};

int fragment(FILE* file_to_send, char* buf){
    int i, len;

    for(i = 0; i < MAX_PACKET_SIZE; i++){
        buf[i] = fgetc(file_to_send);
        if(buf[i] == EOF){
            return 1;
        }
    }
    return 0;
}

int findSize(char* filename){
    FILE* fp = fopen(filename, "r");   
    fseek(fp, 0L, SEEK_END);
    int res = ftell(fp);
    fclose(fp);
    return res;
}

int addIntToStr(char* inputStr, int num){
    int size = (int)((ceil(log10(num))+1)*sizeof(char));
    char str[size];
    sprintf(str, "%d", num);
    strcat(inputStr, str);
    return 0;
}

char* formString(struct packet packet){
    char *packetString;
    addIntToStr(packetString, packet.total_frag);
    strcat(packetString,":");
    addIntToStr(packetString, packet.frag_no);
    strcat(packetString,":");
    addIntToStr(packetString,packet.size);
    strcat(packetString,":");
    strcat(packetString,packet.filename);
    strcat(packetString,":");
    
    return packetString;
}

char* stringToBinary(char* s) {
    if(s == NULL) return 0; /* no input string */
    size_t len = strlen(s);
    char *binary = malloc(len*8 + 1); // each char is one byte (8 bits) and + 1 at the end for null terminator
    binary[0] = '\0';
    for(size_t i = 0; i < len; ++i) {
        char ch = s[i];
        for(int j = 7; j >= 0; --j){
            if(ch & (1 << j)) {
                strcat(binary,"1");
            } else {
                strcat(binary,"0");
            }
        }
    }
    return binary;
}

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
    char* binaryString; 

    time_t begin, end;

    FILE* file_to_send;
    struct packet fileTranPacket;
    char* packetString;
    int n = 1;

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

    fileTranPacket.filename = filename;
    fileTranPacket.size = findSize(filename);
    fileTranPacket.total_frag = (fileTranPacket.size + MAX_PACKET_SIZE - 1) / MAX_PACKET_SIZE;

    while(n < fileTranPacket.total_frag){//packet format: "total_frag:frag_no:size:filename:filedata"
        fileTranPacket.frag_no = n;
        fragment(file_to_send, fileTranPacket.filedata);
        packetString = formString(fileTranPacket);//fileTranPacket.total_frag + ":" + fileTranPacket.frag_no + ":" + fileTranPacket.size + ":" + fileTranPacket.filedata;
        binaryString = stringToBinary(packetString);
        n++;
    }
    
    close(sock);
    return 0;
}