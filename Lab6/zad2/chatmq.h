#ifndef CHAT_MQ_H
#define CHAT_MQ_H
#include "message.h"
#define MAX_CLIENTS 10
#define SERVER_QUEUE "/server_queue"

char *getClientQueueName();
int send(mqd_t mq, message_t *msg);
int receive(mqd_t mq, message_t *msg);
mqd_t createQueue(char *qName);
mqd_t getQueue(char *qName);
int closeQueue(mqd_t mq);
int unlinkQueue(char *name);
#endif