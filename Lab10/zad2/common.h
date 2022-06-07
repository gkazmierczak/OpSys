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
#define MAX_MSG_LEN 32

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
    struct sockaddr *addr;

} client;

typedef struct message
{
    messageType type;
    char *data;
    char *name;
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

void sendMsg(int fd, messageType type, char *content, struct sockaddr *addr)
{
    char *data = (char *)calloc(MAX_MSG_LEN, sizeof(char));
    sprintf(data, "%d|%s", (int)type, content);
    fflush(stdout);
    err(sendto(fd, data, MAX_MSG_LEN, 0, addr, sizeof(struct sockaddr)), -1, "Could not send message.");
    free(data);
}
void sendMsgFd(int fd, messageType type, char *content, char *name)
{
    char *data = (char *)calloc(MAX_MSG_LEN, sizeof(char));
    sprintf(data, "%d|%s|%s", (int)type, content, name);
    fflush(stdout);
    err(send(fd, data, MAX_MSG_LEN, 0), -1, "Could not send message.");
    free(data);
}
message *readMsg(int fd, struct sockaddr *addr, socklen_t *addrlen)
{
    message *msg = (message *)malloc(sizeof(message));
    char *content = (char *)calloc(MAX_MSG_LEN, sizeof(char));
    int bytesRead = recvfrom(fd, (void *)content, MAX_MSG_LEN, 0, addr, addrlen);

    err(bytesRead, -1, "Message read error.");
    if (bytesRead > 0)
    {
        char *type = strtok(content, "|");
        char *data = strtok(NULL, "|");
        char *name = strtok(NULL, "|");
        msg->type = atoi(type);
        msg->data = data;
        msg->name = name;
        return msg;
    }
    else
    {
        free(content);
        return NULL;
    }
}
