#include "chatmq.h"

#include <stdlib.h>
#include <mqueue.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

mqd_t createQueue(char *qName)
{
    struct mq_attr atr;
    atr.mq_flags = 0;
    atr.mq_maxmsg = 10;
    atr.mq_msgsize = sizeof(message_t);
    atr.mq_curmsgs = 0;
    mqd_t mq = mq_open(qName, O_CREAT | O_RDWR, 0644, &atr);
    return mq;
}
mqd_t getQueue(char *qName)
{
    mqd_t mq = mq_open(qName, O_RDWR);
    return mq;
}

int closeQueue(mqd_t mq)
{
    return mq_close(mq);
}

int send(mqd_t mq, message_t *msg)
{
    return mq_send(mq, (const char *)msg, sizeof(message_t), msg->type);
}

int receive(mqd_t mq, message_t *msg)
{
    return mq_receive(mq, (char *)msg, sizeof(message_t), NULL);
}
char *getClientQueueName()
{
    char *queueName = (char *)calloc(32, sizeof(char));
    sprintf(queueName, "/queue%d", getpid());
    return queueName;
}
int unlinkQueue(char *name)
{
    return mq_unlink(name);
}