#ifndef NETWORK_H
#define NETWORK_H

#include <easyobjects.h>
#include <easybool.h>
#include <easynet.h>

#define MAX_MESSAGE_SIZE 2048
#define MAX_USERNAME_SIZE 18
#define SIGMSG_PORT 9876

typedef enum {
    MESSAGE_PACKET = 0,
} Header;

typedef struct {
    uint64_t first;
    uint64_t second;
} UUID;

typedef struct {
    uint8_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t zone;
} Timestamp;

typedef struct {
    Header type;
    UUID from;
    UUID to;
    Timestamp time;
    uint16_t size;
    char text[MAX_MESSAGE_SIZE]; // IMPORTANT: this should be last
} Message;

DECLARE_ARRLIST(Message);

typedef struct {
    UUID id;
    BOOL unread;
    ARRLIST_Message history;
    char name[MAX_USERNAME_SIZE];
} User;

DECLARE_ARRLIST(User);

DECLARE_ARRLIST_NAMED(ez_ConnectionPtr, ez_Connection*);

typedef struct {
    ez_Client* client;
    User* user;
} LinkedClient;

DECLARE_ARRLIST(LinkedClient);

typedef struct {
    User* user;
    Message* message;
} QueuedMessage;

DECLARE_ARRLIST(QueuedMessage);

typedef struct {
    UUID id;
    ARRLIST_User friends;
} Network;

UUID GenerateUUID();

void InitializeNetwork();

Network* NetworkRef();

void CleanNetwork();

void SendChat(User* user, const char* chat);

#endif