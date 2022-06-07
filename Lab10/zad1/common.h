#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/un.h>
#include <pthread.h>
#include <poll.h>
#include <signal.h>

#define BOARD_SIZE 9
#define MAX_CLIENTS 16
#define MAX_MSG_LEN 16

typedef enum messageType
{
    MSG_LOGIN,
    MSG_DISCONNECT,
    MSG_PING,
    MSG_MOVE,
    MSG_START,
    MSG_GAME_END,
    MSG_ERR
} messageType;

typedef struct client
{
    char name[MAX_MSG_LEN + 1];
    int fd;
    int status;
    int opponent;

} client;

typedef struct message
{
    messageType type;
    char *data;
} message;

void err(int val, int eq, char *msg)
{
    if (val == eq)
    {
        fprintf(stderr, "ERROR: %s\n", msg);
        perror("    ");
        fflush(stderr);
        exit(EXIT_FAILURE);
    }
}

void sendMsg(int fd, messageType type, char *content)
{
    char *data = (char *)calloc(MAX_MSG_LEN, sizeof(char));
    sprintf(data, "%d|%s", (int)type, content);
    fflush(stdout);
    err(send(fd, data, MAX_MSG_LEN, 0), -1, "Could not send message.");
    free(data);
}

message *readMsg(int fd)
{
    message *msg = (message *)malloc(sizeof(message));
    char *content = (char *)calloc(MAX_MSG_LEN, sizeof(char));
    char *data = (char *)calloc(MAX_MSG_LEN, sizeof(char));
    int bytesRead = recv(fd, (void *)content, MAX_MSG_LEN, 0);
    err(bytesRead, -1, "Message read error.");
    if (bytesRead > 0)
    {
        msg->type = atoi(&content[0]);
        strncpy(data, &content[2], MAX_MSG_LEN);
        free(content);
        msg->data = data;
        return msg;
    }
    else
    {
        free(content);
        free(data);
        return NULL;
    }
}