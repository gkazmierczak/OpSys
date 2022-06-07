#include "common.h"

int port, localSocketFd, inetSocketFd;
char *socketPath;
client *clients[MAX_CLIENTS] = {NULL};
pthread_mutex_t clientsMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t serverThread;
pthread_t pingThread;
void disconnectClient(int fd, int index);

void unregisterClient(int index)
{
    if (clients[index]->opponent != -1)
    {
        clients[clients[index]->opponent]->opponent = -1;
        disconnectClient(clients[clients[index]->opponent]->fd, clients[index]->opponent);
    }
    clients[index] = NULL;
}
void disconnectClient(int fd, int index)
{
    sendMsg(fd, MSG_DISCONNECT, "disconnect");
    close(clients[index]->fd);
    unregisterClient(index);
}
void handleMove(int index, message *msg)
{
    int opponentFd = clients[clients[index]->opponent]->fd;
    sendMsg(opponentFd, MSG_MOVE, msg->data);
}
void handleMsg(int fd, int index)
{
    message *msg = readMsg(fd);
    if (msg != NULL)
    {
        switch (msg->type)
        {
        case MSG_DISCONNECT:
            unregisterClient(index);
            break;
        case MSG_PING:
            clients[index]->status = 1;
            break;
        case MSG_MOVE:
            handleMove(index, msg);
            break;
        default:
            break;
        }
    }
    free(msg);
}
void addClient(int fd)
{
    int clientFd = accept(fd, NULL, NULL);
    err(clientFd, -1, "Accept failure.");
    message *msg = readMsg(clientFd);

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
        sendMsg(clientFd, MSG_ERR, "name taken");
    }
    else if (index == -1)
    {
        puts("Client connection refused : Max clients reached");
        sendMsg(clientFd, MSG_ERR, "max clients");
    }
    else if (index >= 0)
    {
        puts("Client connected");
        client *newClient = (client *)calloc(1, sizeof(client));
        strcpy(newClient->name, msg->data);
        newClient->fd = clientFd;
        newClient->opponent = opponent;
        newClient->status = 1;
        clients[index] = newClient;
        sendMsg(clientFd, MSG_LOGIN, NULL);
        if (opponent != -1)
        {
            clients[opponent]->opponent = index;
            int x = rand() % 10;
            if (x > 4)
            {
                sendMsg(clients[opponent]->fd, MSG_START, "X");
                sendMsg(clientFd, MSG_START, "O");
                sendMsg(clients[opponent]->fd, MSG_MOVE, "INIT");
            }
            else
            {
                sendMsg(clients[opponent]->fd, MSG_START, "O");
                sendMsg(clientFd, MSG_START, "X");
                sendMsg(clientFd, MSG_MOVE, "INIT");
            }
        }
    }

    free(msg);
}
int setupLocalSocket(char *socketPath)
{
    int status;
    int socketFd = socket(AF_LOCAL, SOCK_STREAM, 0);
    err(socketFd, -1, "Could not create LOCAL socket.");
    struct sockaddr_un saddr = {.sun_family = AF_UNIX};
    sprintf(saddr.sun_path, "%s", socketPath);
    unlink(socketPath);
    status = bind(socketFd, (struct sockaddr *)&saddr, sizeof(saddr));
    err(status, -1, "Could not bind LOCAL socket.");
    status = listen(socketFd, MAX_CLIENTS);
    err(status, -1, "Failed to listen on LOCAL socket.");
    return socketFd;
}

int setupNetworkSocket(int port)
{
    int status;
    int socketFd = socket(AF_INET, SOCK_STREAM, 0);
    err(socketFd, -1, "Could not create INET socket.");
    struct sockaddr_in saddr = {.sin_addr.s_addr = inet_addr("127.0.0.1"), .sin_family = AF_INET, .sin_port = htons(port)};
    status = bind(socketFd, (struct sockaddr *)&saddr, sizeof(saddr));
    err(status, -1, "Could not bind INET socket.");
    status = listen(socketFd, MAX_CLIENTS);
    err(status, -1, "Failed to listen on INET socket.");
    return socketFd;
}

void *serverLoop(void *arg)
{
    struct pollfd *pollfds = calloc(MAX_CLIENTS + 2, sizeof(struct pollfd));
    pollfds[MAX_CLIENTS].fd = localSocketFd;
    pollfds[MAX_CLIENTS].events = POLLIN;
    pollfds[MAX_CLIENTS + 1].fd = inetSocketFd;
    pollfds[MAX_CLIENTS + 1].events = POLLIN;
    while (1)
    {
        pthread_mutex_lock(&clientsMutex);
        for (int i = 0; i < MAX_CLIENTS; i++)
        {
            if (clients[i] != NULL)
            {
                pollfds[i].fd = clients[i]->fd;
            }
            else
            {
                pollfds[i].fd = -1;
            }
            pollfds[i].events = POLLIN;
            pollfds[i].revents = 0;
        }
        pthread_mutex_unlock(&clientsMutex);

        pollfds[MAX_CLIENTS].revents = 0;
        pollfds[MAX_CLIENTS + 1].revents = 0;

        poll(pollfds, MAX_CLIENTS + 2, -1);

        pthread_mutex_lock(&clientsMutex);
        for (int i = 0; i < MAX_CLIENTS + 2; i++)
        {
            if (pollfds[i].revents && pollfds[i].events == POLLIN)
            {
                if (i >= MAX_CLIENTS)
                {
                    addClient(pollfds[i].fd);
                }
                else if (clients[i] != NULL)
                {
                    handleMsg(pollfds[i].fd, i);
                }
            }
            else if (pollfds[i].revents & POLLHUP)
            {
                disconnectClient(pollfds[i].fd, i);
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
                sendMsg(clients[i]->fd, MSG_PING, NULL);
            }
            else if (clients[i] != NULL && clients[i]->status == 0)
            {
                disconnectClient(clients[i]->fd, i);
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
            disconnectClient(clients[i]->fd, i);
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
    localSocketFd = setupLocalSocket(socketPath);
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
