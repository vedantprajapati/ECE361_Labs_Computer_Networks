#ifndef HELPERS_H
#define HELPERS_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

struct message
{
    int type;
    int size;
    char source[32];
    char data[1048];
};

struct user
{
    char username[32];
    char password[32];
    char session_id[32];
    int sock_fd;
};

struct packet
{                            // packet format: "total_frag:frag_no:size:filename:filedata"
    unsigned int total_frag; // total number of fragments of the file
    unsigned int frag_no;    // sequence number of fragment
    unsigned int size;       // size of data, range [0,1000]
    char *filename;
    char filedata[1048];
};

struct session
{
    char id[32];
    int activeUsers;
};

struct sessions
{
    struct session *session;
    struct sessions *next;
};

struct users
{
    struct user *array;
    size_t len;
    size_t capacity;
};

enum types
{
    LOGIN,
    LO_ACK,
    LO_NAK,
    EXIT,
    JOIN,
    JN_ACK,
    JN_NAK,
    LEAVE_SESS,
    NEW_SESS,
    NS_ACK,
    MESSAGE,
    QUERY,
    QU_ACK
};

// create a packet from a message by the client
void convert_client_input_to_packet(char *str, struct message *message);

// display a packet as a string
void display_packet(struct message *message);

//convert the packet to a string to be sent to the reciever
void display_message(char *buffer, long int type, int length, char *sender, char *text);

// check if user is in the list of users
struct user *lookup_user_name(struct users *user, char *name);

// add user to currently logged in users
void add_user_id(struct users *users, struct user *new_user);

// remove user with name from currently logged in users
void rm_user_id(struct users *users, char *name);

// check database for corresponding user
struct user *lookup_user_creds(char *usr_name, char *file_name);

// find session by id
struct session *lookup_session(struct sessions *sessions, char *id);

// remove session from sessions
void rm_session(struct sessions *sessions, struct session *session);

// add a new session to head of current sessions list
struct session *add_session(struct sessions *sessions, char *session_name);

bool add_user_to_session(struct users *users, struct sessions *sessions, struct session *session, struct user *user);

bool rm_user_from_session(struct users *users, struct sessions *sessions, struct session *session, struct user *user);

#endif /* HELPERS_H */