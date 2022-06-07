#include "common.h"

int port, localSocketFd, inetSocketFd;
char *socketPath;
client *clients[MAX_CLIENTS] = {NULL};
pthread_mutex_t clientsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t serverThread;
pthread_t pingThread;
void unregisterClient(int index);
int findClient(char *name)
{
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] != NULL && !strcmp(name, clients[i]->name))
        {
            return i;
        }
    }
    return -1;
}
void disconnectClient(char *name)
{
    int index = findClient(name);
    sendMsg(clients[index]->fd, MSG_DISCONNECT, "disconnect", clients[index]->addr);
    unregisterClient(index);
}
void unregisterClient(int index)
{
    if (clients[index]->opponent != -1)
    {
        clients[clients[index]->opponent]->opponent = -1;
        disconnectClient(clients[clients[index]->opponent]->name);
    }
    free(clients[index]);
    clients[index] = NULL;
    puts("Client disconnected");
}

void handleMove(message *msg)
{
    int index = findClient(msg->name);
    int opponentFd = clients[clients[index]->opponent]->fd;
    sendMsg(opponentFd, MSG_MOVE, msg->data, clients[clients[index]->opponent]->addr);
}
void addClient(int fd, struct sockaddr *addr, message *msg)
{
    int index = -1;
    int opponent = -1;
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] == NULL && index < 0)
        {
            index = i;
        }
        if (clients[i] != NULL && strcmp(clients[i]->name, msg->data) == 0)
        {
            printf("id %d  n1 %s n2 %s\n", i, clients[i]->name, msg->data);
            fflush(stdout);
            index = -2;
            break;
        }
        if (clients[i] != NULL && clients[i]->opponent == -1)
        {
            opponent = i;
        }
    }
    if (index == -2)
    {
        puts("Client connection refused : Name taken");
        sendMsg(fd, MSG_ERR, "name taken", addr);
    }
    else if (index == -1)
    {
        puts("Client connection refused : Max clients reached");
        sendMsg(fd, MSG_ERR, "max clients", addr);
    }
    else if (index >= 0)
    {
        puts("Client connected");
        client *newClient = (client *)calloc(1, sizeof(client));
        strcpy(newClient->name, msg->data);
        newClient->fd = fd;
        newClient->opponent = opponent;
        newClient->status = 1;
        newClient->addr = addr;
        clients[index] = newClient;
        sendMsg(fd, MSG_LOGIN, NULL, addr);
        if (opponent != -1)
        {
            clients[opponent]->opponent = index;
            int x = rand() % 10;
            if (x > 4)
            {
                sendMsg(clients[opponent]->fd, MSG_START, "X", clients[opponent]->addr);
                sendMsg(fd, MSG_START, "O", addr);
                sendMsg(clients[opponent]->fd, MSG_MOVE, "INIT", clients[opponent]->addr);
            }
            else
            {
                sendMsg(clients[opponent]->fd, MSG_START, "O", clients[opponent]->addr);
                sendMsg(fd, MSG_START, "X", addr);
                sendMsg(fd, MSG_MOVE, "INIT", addr);
            }
        }
    }
}
void handleMsg(int fd)
{
    struct sockaddr *addr = (struct sockaddr *)malloc(sizeof(struct sockaddr));
    socklen_t len = sizeof(&addr);
    message *msg = readMsg(fd, addr, &len);
    int index;
    if (msg != NULL)
    {
        switch (msg->type)
        {
        case MSG_LOGIN:
            addClient(fd, addr, msg);
            break;
        case MSG_DISCONNECT:
            index = findClient(msg->name);
            unregisterClient(index);
            break;
        case MSG_PING:
            index = findClient(msg->name);
            clients[index]->status = 1;
            break;
        case MSG_MOVE:
            handleMove(msg);
            break;
        default:
            break;
        }
    }
    free(msg);
}

int setupLocalSocket(char *socketPath)
{
    int status;
    int socketFd = socket(AF_LOCAL, SOCK_DGRAM, 0);
    err(socketFd, -1, "Could not create LOCAL socket.");
    struct sockaddr_un saddr = {.sun_family = AF_UNIX};
    sprintf(saddr.sun_path, "%s", socketPath);
    unlink(socketPath);
    status = bind(socketFd, (struct sockaddr *)&saddr, sizeof(saddr));
    err(status, -1, "Could not bind LOCAL socket.");
    return socketFd;
}

int setupNetworkSocket(int port)
{
    int status;
    int socketFd = socket(AF_INET, SOCK_DGRAM, 0);
    err(socketFd, -1, "Could not create INET socket.");
    struct sockaddr_in saddr = {.sin_addr.s_addr = inet_addr("127.0.0.1"), .sin_family = AF_INET, .sin_port = htons(port)};
    status = bind(socketFd, (struct sockaddr *)&saddr, sizeof(saddr));
    err(status, -1, "Could not bind INET socket.");
    return socketFd;
}

void *serverLoop(void *arg)
{
    struct pollfd *pollfds = calloc(2, sizeof(struct pollfd));
    pollfds[0].fd = localSocketFd;
    pollfds[0].events = POLLIN;
    pollfds[1].fd = inetSocketFd;
    pollfds[1].events = POLLIN;
    while (1)
    {

        pollfds[0].revents = 0;
        pollfds[1].revents = 0;

        poll(pollfds, 2, -1);

        pthread_mutex_lock(&clientsMutex);
        for (int i = 0; i < 2; i++)
        {
            if (pollfds[i].revents && pollfds[i].events == POLLIN)
            {
                handleMsg(pollfds[i].fd);
            }
        }
        pthread_mutex_unlock(&clientsMutex);
    }
    pthread_exit(NULL);
}
void *pingLoop(void *arg)
{
    while (1)
    {
        sleep(10);
        pthread_mutex_lock(&clientsMutex);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i] != NULL && clients[i]->status == 1)
            {
                clients[i]->status = 0;
                sendMsg(clients[i]->fd, MSG_PING, NULL, clients[i]->addr);
            }
            else if (clients[i] != NULL && clients[i]->status == 0)
            {
                disconnectClient(clients[i]->name);
            }
        }
        pthread_mutex_unlock(&clientsMutex);
    }
}

void terminateServer()
{
    puts("SIGINT received, terminating server and closing all sockets.");
    for (int i = 0; i < MAX_CLIENTS; i++)
    {
        if (clients[i] != NULL)
        {
            disconnectClient(clients[i]->name);
        }
    }
    pthread_cancel(serverThread);
    pthread_cancel(pingThread);
    exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        puts("Incorrect arguments. Usage: ./server PORT PATH");
        exit(EXIT_FAILURE);
    }
    else
    {
        port = atoi(argv[1]);
        socketPath = argv[2];
    }
    // localSocketFd = setupLocalSocket(socketPath);
    signal(SIGINT, terminateServer);
    inetSocketFd = setupNetworkSocket(port);
    int status;
    status = pthread_create(&serverThread, NULL, serverLoop, NULL);
    err(status, -1, "Could not create thread for server.");
    status = pthread_create(&pingThread, NULL, pingLoop, NULL);
    err(status, -1, "Could not create pinging thread for server.");
    pthread_join(serverThread, NULL);
    pthread_join(pingThread, NULL);
}
