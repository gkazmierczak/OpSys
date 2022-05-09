#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include "chatmq.h"
#include "message.h"

int clients[MAX_CLIENTS][2];
mqd_t serverQueueID;
int currentClientCount = 0;
int nextClientID = 0;
FILE *file;
message_t msgIn;
message_t msgOut;

void logToFile(message_t *msg)
{
    struct tm *timeinfo;
    timeinfo = localtime(&msg->timestamp);
    fprintf(file, "TIME: %s  |  Client ID: %d  |  Message content: %s \n", asctime(timeinfo), msg->senderID, msg->content);
}
void cleanup()
{
    int status = 0;
    if (currentClientCount > 0)
    {
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i][0] != -1)
            {
                msgOut.type = MSG_STOP;
                status = send(clients[i][1], &msgOut);
                if (status == -1)
                {
                    perror("Cannot send STOP message: ");
                }
            }
        }
    }
    closeQueue(serverQueueID);
    unlinkQueue(SERVER_QUEUE);
    time_t timestamp = time(NULL);
    struct tm *timeinfo;
    timeinfo = localtime(&timestamp);
    fprintf(file, "SERVER SHUTDOWN at %s\n", asctime(timeinfo));
    fclose(file);
    puts("\nExiting.");
}
void handleSig(int sig)
{
    exit(0);
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

void handleClientDisconnect(message_t *msg)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i][0] == msg->senderID)
        {
            closeQueue(clients[i][1]);
            clients[i][0] = -1;
            clients[i][1] = -1;
            currentClientCount--;
            printf("Successfully closed client %d\n", msg->senderID);
            break;
        }
    }
}
void handleList(message_t *msg)
{
    puts("---------------- Active clients ----------------");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i][0] != -1)
        {
            printf("Client ID: %d  | Queue ID: %d\n", clients[i][0], clients[i][1]);
        }
    }
}
void handleMessageSend(message_t *msg)
{
    if (msg->targetID != -1)
    {
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i][0] == msg->targetID)
            {
                send(clients[i][1], msg);
                break;
            }
        }
    }
}
void handleBroadcastMessage(message_t *msg)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i][0] != -1 && clients[i][0] != msg->senderID)
        {
            send(clients[i][1], msg);
        }
    }
}

int addClient(int clientQueueID)
{
    if (currentClientCount >= MAX_CLIENTS)
    {
        return -1;
    }
    currentClientCount++;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i][0] == -1)
        {
            clients[i][0] = nextClientID;
            clients[i][1] = clientQueueID;
            break;
        }
    }
    return nextClientID++;
}
void handleInit(message_t *msg)
{
    char *clientQueueName = msg->content;
    mqd_t clientQueueID = getQueue(clientQueueName);
    if (clientQueueID == -1)
    {
        perror("ERROR: Could not open client queue");
    }
    else if (currentClientCount < MAX_CLIENTS)
    {
        int clientID = addClient(clientQueueID);
        message_t msgr = {.type = MSG_INIT,
                          .targetID = clientID,
                          .senderID = -1};
        send(clientQueueID, &msgr);
    }
    else
    {
        fprintf(stderr, "ERROR: Cannot add client - Maximum client count reached");
    }
}

void init()
{
    serverQueueID = createQueue(SERVER_QUEUE);
    if (serverQueueID == -1)
    {
        perror("ERROR: Could not open server queue");
        exit(-1);
    }
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        clients[i][0] = -1;
    }
    file = fopen("./logfile.log", "a+");
    if (file == NULL)
    {
        perror("Could not open logfile");
        exit(-1);
    }
    signal(SIGINT, handleSig);
    time_t timestamp = time(NULL);
    struct tm *timeinfo;
    timeinfo = localtime(&timestamp);
    fprintf(file, "SERVER STARTUP at %s\n", asctime(timeinfo));
    printf("Server running, server queue id: %d\n", serverQueueID);
}

int main(void)
{
    atexit(cleanup);
    init();
    while (1)
    {
        if (receive(serverQueueID, &msgIn) == -1)
        {
            if (errno != EINTR)
            {
                perror("Could not receive message");
                exit(-1);
            }
        }
        logToFile(&msgIn);
        switch (msgIn.type)
        {
        case MSG_STOP:
            handleClientDisconnect(&msgIn);
            break;
        case MSG_LIST:
            handleList(&msgIn);
            break;
        case MSG_INIT:
            handleInit(&msgIn);
            break;
        case MSG_ONE:
            handleMessageSend(&msgIn);
            break;
        case MSG_ALL:
            handleBroadcastMessage(&msgIn);
            break;
        default:
            fprintf(stderr, "Wrong message type\n");
            break;
        }
    }

    return 0;
}