#ifndef HELPERS_H
#define HELPERS_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>

struct message {
    int type;
    int size;
    unsigned source[32];
    unsigned data[1048];
};

struct user {
    char username[32];
    char password[32];
    char session_id[32];
    int sock_fd;
};

struct sessions{
    char id[32];
    int activeUsers;
};

struct sessions_list{
    struct sessions *session;
    struct sessions_list *next;
};


struct users{
    struct user *user;
    struct users *next;
};

enum types {
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

void convert_str_to_packet(char* str, struct message *message);

void display_packet(struct message *message);

struct user* lookup_user_name(struct users *user, char* name);

void add_user_id(struct users *users, struct user *new_user);

void rm_user_id(struct users *users, char* name);

struct user* lookup_user_creds(char* usr_name, char* file_name);

struct sessions* lookup_session(struct sessions_list * sessions, char* session);

void rm_session(struct sessions_list * sessions, struct sessions *session);

// add a new session to head of current sessions list
struct sessions* add_session(struct sessions_list *s_list, char* session_name);

bool add_user_to_session(struct users *users, struct sessions_list *sessions, struct sessions *session, struct user *user);

bool rm_user_from_session(struct users *users, struct sessions_list *sessions, struct sessions *session, struct user *user);

#endif /* HELPERS_H */