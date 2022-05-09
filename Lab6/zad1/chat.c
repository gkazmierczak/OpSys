#include "chat.h"

#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <stdio.h>

int createQueue(key_t key)
{
    int qID = msgget(key, IPC_CREAT | IPC_EXCL | 0600);
    return qID;
}
int getQueue(key_t key)
{
    int qID = msgget(key, 0);
    return qID;
}

int closeQueue(int queue)
{
    return msgctl(queue, IPC_RMID, NULL);
}

int send(int queue, message_t *msg)
{
    return msgsnd(queue, msg, sizeof(message_t) - sizeof(long), 0);
}

int receive(int queue, message_t *msg)
{
    return msgrcv(queue, msg, sizeof(message_t) - sizeof(long), -MSG_ERR, 0);
}
int receive_nowait(int queue, message_t *msg)
{
    return msgrcv(queue, msg, sizeof(message_t) - sizeof(long), -MSG_ERR, IPC_NOWAIT);
}
key_t getServerKey()
{
    key_t key;
    if ((key = ftok(getenv("HOME"), 1)) == -1)
    {
        perror("Could not obtain server key");
    }
    return key;
}
key_t getClientKey()
{
    key_t key;
    if ((key = ftok(getenv("HOME"), getpid())) == -1)
    {
        perror("Could not obtain client key");
    }
    return key;
}