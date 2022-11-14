
#ifndef USER_H
#define USER_H

struct user {
    char *username;
    char *password;
    bool active;
    char* ip;
    char* port;
    char* session_id;
};

#endif /* USER_H */