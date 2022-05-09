#ifndef MESSAGE_H
#define MESSAGE_H

#include <sys/types.h>

#define MAX_MSG_SIZE 4096
#define MSG_Q_SIZE 10

typedef enum MsgType
{
    MSG_STOP = 1,
    MSG_LIST = 2,
    MSG_INIT = 3,
    MSG_ALL = 4,
    MSG_ONE = 5,
    MSG_ERR = 6
} MsgType;

struct Message
{
    long type;
    time_t timestamp;
    int targetID;
    int senderID;
    char content[MAX_MSG_SIZE];
};
typedef struct Message message_t;

#endif