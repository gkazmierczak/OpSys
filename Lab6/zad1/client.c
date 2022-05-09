#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "chat.h"
#include "message.h"

//  do POSIX DODAĆ W MAKEFILU -lrt przy kompilacji

int serverQueueID = -1;
int queueID = -1;
int assignedID = -1;
pid_t parentPid = -1;
pid_t childPid = -1;
char *inputBuffer = NULL;
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
            msgctl(queueID, IPC_RMID, NULL);
        }
        if (childPid > 0)
        {
            kill(childPid, SIGKILL);
        }
        // if (inputBuffer != NULL)
        // {
        //     free(inputBuffer);
        // }
    }
}
void handleSig(int sig)
{

    // puts("\nSIGINT Received, terminating.");
    exit(0);
}

void initClient(key_t key)
{
    msgOut.type = MSG_INIT;
    msgOut.timestamp = time(NULL);
    sprintf(msgOut.content, "%d", key);
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
    int targetID;
    while (1)
    {
        messageType = "";
        printf("> ");
        fgets(inputBuffer, MAX_MSG_SIZE + 31, stdin);
        messageType = strtok_r(inputBuffer, " ", &inputBuffer);
        messageType[strcspn(messageType, "\n")] = 0;
        msgOut.senderID = assignedID;
        msgOut.timestamp = time(NULL);
        MsgType msgType = msgTypeFromString(messageType);
        switch (msgType)
        {
        case MSG_STOP:
            puts("Exiting.");
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
            msgOut.targetID = -1;
            msgOut.type = MSG_ALL;
            content = strtok_r(inputBuffer, " ", &inputBuffer);
            strncpy(msgOut.content, content, MAX_MSG_SIZE - 1);
            send(serverQueueID, &msgOut);
            break;

        case MSG_ONE:
            target = strtok_r(inputBuffer, " ", &inputBuffer);
            targetID = atoi(target);
            msgOut.senderID = assignedID;
            msgOut.targetID = targetID;
            msgOut.type = MSG_ONE;
            content = strtok_r(inputBuffer, "", &inputBuffer);
            strncpy(msgOut.content, content, MAX_MSG_SIZE - 1);
            send(serverQueueID, &msgOut);
            break;
        default:
            fprintf(stderr, "Incorrect command.\n");
            break;
        }
    }
    free(inputBuffer);
    inputBuffer = NULL;
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
            // exit(0);
            break;
        default:
            printMessage(&msgIn);
            break;
        }
    }
}

int main(void)
{
    // msgOut = (message_t *)calloc(1, sizeof(message_t));
    // msgIn = (message_t *)calloc(1, sizeof(message_t));

    atexit(handleExit);
    key_t serverQueueKey = getServerKey();
    serverQueueID = getQueue(serverQueueKey);
    if (serverQueueID == -1)
    {
        perror("ERROR: Could not open server queue");
        exit(-1);
    }
    printf("Server queue: %d\n", serverQueueID);
    key_t clientQueueKey = getClientKey();
    queueID = createQueue(clientQueueKey);
    if (queueID == -1)
    {
        perror("ERROR: Could not open client queue");
        exit(-1);
    }
    initClient(clientQueueKey);
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