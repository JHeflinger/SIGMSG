#ifndef NETWORK_H
#define NETWORK_H

#include <easyobjects.h>
#include <easybool.h>
#include <easynet.h>

#define MAX_MESSAGE_SIZE 128
#define MAX_USERNAME_SIZE 18
#define SIGMSG_PORT 9876
#define MAX_SEND_ATTEMPTS 3

typedef enum {
    MESSAGE_PACKET = 0,
    REGISTER_PACKET = 1,
    CONNECT_PACKET = 2,
    ACK_PACKET = 3,
    PEER_PACKET = 4,
    FAILURE_PACKET = 5,
	PUNCH_PACKET = 6,
	FIST_PACKET = 7,
    TRANSLATE_PACKET = 8
} Header;

typedef struct {
    uint64_t first;
    uint64_t second;
} UUID;

typedef struct {
	Header type;
} FistPacket;

typedef struct {
	Header type;
	Destination destination;
} PunchPacket;

typedef struct {
    Header type;
    UUID id;
} AckPacket;

typedef struct {
	Header type;
	Destination translation;
} TranslatePacket;

typedef struct {
    Header type;
    UUID peer;
	Destination private_dest;
} RegisterPacket;

typedef struct {
	Header type;
	UUID to;
} ConnectPacket;

typedef struct {
	Header type;
	Destination destination;
	Destination private_dest;
} PeerPacket;

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
    UUID id;
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

typedef struct {
    Destination destination;
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
    BOOL online;
} Network;

UUID GenerateUUID();

void InitializeNetwork();

Network* NetworkRef();

void CleanNetwork();

void SendChat(User* user, const char* chat);

#endif
