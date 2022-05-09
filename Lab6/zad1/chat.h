#ifndef CHAT_H
#define CHAT_H
#include "message.h"
#define MAX_CLIENTS 100

key_t getServerKey();
key_t getClientKey();
int send(int queue, message_t *msg);
int receive(int queue, message_t *msg);
int createQueue(key_t key);
int getQueue(key_t key);
int closeQueue(int queue);
int receive_nowait(int queue, message_t *msg);
#endif