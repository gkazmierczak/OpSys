#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "chatmq.h"
#include "message.h"

mqd_t serverQueueID = -1;
mqd_t queueID = -1;
int assignedID = -1;
pid_t parentPid = -1;
pid_t childPid = -1;
char *inputBuffer = NULL;
char *queueName = NULL;
message_t msgIn;
message_t msgOut;

void handleExit()
{
    // if queue was initialised
    if (getpid() == parentPid)
    {
        if (queueID != -1)
        {
            msgOut.timestamp = time(NULL);
            msgOut.type = MSG_STOP;
            sprintf(msgOut.content, "STOP");
            msgOut.senderID = assignedID;
            msgOut.targetID = -1;
            send(serverQueueID, &msgOut);
            closeQueue(queueID);
            unlinkQueue(queueName);
        }
        if (childPid > 0)
        {
            kill(childPid, SIGKILL);
        }
        if (queueName != NULL)
        {
            free(queueName);
        }
    }
}
void handleSig(int sig)
{
    exit(0);
}

void initClient(char *queueName)
{
    msgOut.type = MSG_INIT;
    msgOut.timestamp = time(NULL);
    sprintf(msgOut.content, "%s", queueName);
    if (send(serverQueueID, &msgOut) == -1)
    {
        perror("ERROR during init: ");
        exit(-1);
    }
    if (receive(queueID, &msgIn) == -1)
    {
        perror("ERROR during init: ");
        exit(-1);
    }
    assignedID = msgIn.targetID;

    printf("Initialised. Assigned Client ID: %d, Queue ID: %d\n", assignedID, queueID);
    return;
}
MsgType msgTypeFromString(char *s)
{
    if (!strcasecmp(s, "STOP"))
    {
        return MSG_STOP;
    }
    if (!strcasecmp(s, "LIST"))
    {
        return MSG_LIST;
    }
    if (!strcasecmp(s, "2ALL"))
    {
        return MSG_ALL;
    }
    if (!strcasecmp(s, "2ONE"))
    {
        return MSG_ONE;
    }
    return MSG_ERR;
}

void sender()
{
    inputBuffer = (char *)calloc(5120, sizeof(char));
    char *messageType;
    char *target;
    char *content;
    char *text;
    int targetID;
    while (1)
    {
        messageType = "";
        printf("> ");
        fgets(inputBuffer, MAX_MSG_SIZE + 31, stdin);
        messageType = strtok_r(inputBuffer, " ", &text);
        if (messageType != NULL)
        {
            messageType[strcspn(messageType, "\n")] = 0;
        }
        else
        {
            messageType = inputBuffer;
        }
        msgOut.senderID = assignedID;
        msgOut.timestamp = time(NULL);
        MsgType msgType = msgTypeFromString(messageType);

        switch (msgType)
        {
        case MSG_STOP:
            puts("Exiting.");
            free(inputBuffer);
            inputBuffer = NULL;
            exit(0);
            break;
        case MSG_LIST:
            msgOut.targetID = -1;
            msgOut.type = MSG_LIST;
            sprintf(msgOut.content, "LIST");
            send(serverQueueID, &msgOut);
            break;
        case MSG_ALL:
            if (text != NULL)
            {

                msgOut.targetID = -1;
                msgOut.type = MSG_ALL;
                content = strtok_r(text, " ", &text);
                if (content != NULL)
                {
                    strncpy(msgOut.content, content, MAX_MSG_SIZE - 1);
                    send(serverQueueID, &msgOut);
                }
                else
                {
                    puts("INCORRECT INPUT");
                    continue;
                }
            }
            break;

        case MSG_ONE:
            if (text != NULL)
            {
                target = strtok_r(text, " ", &text);
                if (target == NULL)
                {
                    puts("INCORRECT INPUT");
                    continue;
                }
                targetID = atoi(target);
                msgOut.senderID = assignedID;
                msgOut.targetID = targetID;
                msgOut.type = MSG_ONE;
                content = strtok_r(text, "", &text);
                if (text != NULL)
                {
                    if (content != NULL)
                    {
                        strncpy(msgOut.content, content, MAX_MSG_SIZE - 1);
                        send(serverQueueID, &msgOut);
                    }
                    else
                    {
                        puts("INCORRECT INPUT");
                        continue;
                    }
                }
            }
            break;
        default:
            fprintf(stderr, "Incorrect command.\n");
            break;
        }
    }
}

void printMessage(message_t *msg)
{
    printf("--------------------------------\n");
    if (msg->type == MSG_ALL)
    {
        printf(" - Message from: %d | To: ALL\n", msg->senderID);
    }
    else
    {
        printf(" - Message from: %d | To: %d\n", msg->senderID, msg->targetID);
    }
    printf("%s\n", msg->content);
    printf("--------------------------------\n");
    fflush(stdout);
}

void receiver()
{
    while (1)
    {
        if (receive(queueID, &msgIn) == -1)
        {
            perror("ERROR: Could not receive message");
        }
        switch (msgIn.type)
        {
        case MSG_STOP:
            puts("STOP received, exiting.");
            kill(parentPid, SIGINT);
            break;
        default:
            printMessage(&msgIn);
            break;
        }
    }
}

int main(void)
{
    atexit(handleExit);
    serverQueueID = getQueue(SERVER_QUEUE);
    if (serverQueueID == -1)
    {
        perror("ERROR: Could not open server queue");
        exit(-1);
    }
    printf("Server queue: %d\n", serverQueueID);
    queueName = getClientQueueName();
    printf("%s\n", queueName);
    queueID = createQueue(queueName);
    if (queueID == -1)
    {
        perror("ERROR: Could not open client queue");
        exit(-1);
    }
    initClient(queueName);
    signal(SIGINT, handleSig);
    parentPid = getpid();
    if ((childPid = fork()) == 0)
    {
        receiver();
    }
    else
    {
        sender();
    }
}