#include "common.h"

char *name, *address;
int isLocal, port;
int serverFd = -1;
char board[BOARD_SIZE];
char sign;
pthread_t gameThread;

void connectToServer()
{
    if (isLocal)
    {
        struct sockaddr_un saddr;
        saddr.sun_family = AF_UNIX;
        strcpy(saddr.sun_path, address);
        serverFd = socket(AF_UNIX, SOCK_DGRAM, 0);
        err(serverFd, -1, "Could not open socket");
        struct sockaddr_un caddr;
        caddr.sun_family = AF_UNIX;
        sprintf(caddr.sun_path, "%s", name);
        err(bind(serverFd, (struct sockaddr *)&caddr, sizeof(caddr)), -1, "Could not bind");
        err(connect(serverFd, (struct sockaddr *)&saddr, sizeof(saddr)), -1, "Could not connect to server");
    }
    else
    {
        int status;
        serverFd = socket(AF_INET, SOCK_DGRAM, 0);
        err(serverFd, -1, "Could not open socket.");
        struct sockaddr_in saddr = {.sin_addr.s_addr = inet_addr(address), .sin_family = AF_INET, .sin_port = htons(port)};
        struct sockaddr_in caddr = {.sin_addr.s_addr = inet_addr(address), .sin_family = AF_INET, .sin_port = 0};
        err(bind(serverFd, (struct sockaddr *)&caddr, sizeof(caddr)), -1, "Could not bind");
        status = connect(serverFd, (struct sockaddr *)&saddr, sizeof(saddr));
        err(status, -1, "Could not connect to server.");
    }
    sendMsgFd(serverFd, MSG_LOGIN, name, name);
}
void disconnect()
{
    if (serverFd != -1)
    {
        sendMsgFd(serverFd, MSG_DISCONNECT, "disconnect", name);
    }
    exit(EXIT_SUCCESS);
}
void initBoard(message *msg)
{
    for (int i = 0; i < BOARD_SIZE; i++)
    {
        board[i] = ' ';
    }
    sign = msg->data[0];
    puts("Starting game...");
}

void displayBoard()
{
    puts("Current board state: ");
    for (int i = 0; i < 3; i++)
    {
        puts("-------------");
        printf("| %c | %c | %c |\n", board[3 * i], board[3 * i + 1], board[3 * i + 2]);
    }
    puts("-------------");
    fflush(stdout);
}

void checkGameStatus()
{
    int empty = 0;
    int ended = 0;
    int win = 0;

    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            if (board[3 * i + j] == ' ')
            {
                empty++;
            }
        }
        if (board[3 * i] == board[3 * i + 1] && board[3 * i] == board[3 * i + 2])
        {
            ended = 1 && (board[3 * i] != ' ');
            win = (board[3 * i] == sign);
        }
        else if (board[i] == board[i + 3] && board[i] == board[i + 6])
        {
            ended = 1 && (board[i] != ' ');
            win = (board[i] == sign);
        }
    }
    if (!ended && (board[0] == board[4] && board[0] == board[8]))
    {
        ended = 1 && (board[0] != ' ');
        win = (board[0] == sign);
    }
    if (!ended && (board[2] == board[4] && board[2] == board[6]))
    {
        ended = 1 && (board[2] != ' ');
        win = (board[2] == sign);
    }
    if (ended)
    {
        if (win)
        {
            sendMsgFd(serverFd, MSG_GAME_END, "VICTORY", name);
            puts("YOU WON!");
        }
        else
        {
            puts("YOU LOST...");
        }
        disconnect();
    }
    else if (empty == 0)
    {
        sendMsgFd(serverFd, MSG_GAME_END, "TIE", name);
        puts("DRAW.");
        disconnect();
    }
}
void waitForMove()
{
    printf("Please enter your move [0-8]: ");
    fflush(stdout);

    char move[MAX_MSG_LEN];
    int pos;
    scanf("%d", &pos);
    while (pos < 0 || pos > 8 || board[pos] != ' ')
    {
        puts("Invalid move. Try again");
        scanf("%d", &pos);
    }
    sprintf(move, "%d", pos);
    board[pos] = sign;
    displayBoard();
    sendMsgFd(serverFd, MSG_MOVE, move, name);
    checkGameStatus();
    puts("Waiting for opponent's move: ");
}
void *handleMove(void *arg)
{
    message *msg = (message *)arg;
    if (strcmp(msg->data, "INIT"))
    {
        if (sign == 'X')
        {
            board[atoi(msg->data)] = 'O';
        }
        else
        {
            board[atoi(msg->data)] = 'X';
        }
    }
    displayBoard();
    checkGameStatus();
    waitForMove();
    free(msg);
    pthread_exit(NULL);
}

void listenToServer()
{
    int status;
    while (1)
    {
        struct sockaddr *addr = (struct sockaddr *)malloc(sizeof(struct sockaddr));
        socklen_t len = sizeof(&addr);
        message *msg = readMsg(serverFd, addr, &len);
        switch (msg->type)
        {
        case MSG_LOGIN:
            puts("Successfully logged in.");
            break;
        case MSG_ERR:
            printf("ERROR: %s\n", msg->data);
            exit(EXIT_SUCCESS);
            break;
        case MSG_DISCONNECT:
            puts("Disconnected from server, terminating...");
            close(serverFd);
            exit(EXIT_SUCCESS);
            break;
        case MSG_PING:
            sendMsgFd(serverFd, MSG_PING, "PONG", name);
            break;
        case MSG_START:
            initBoard(msg);
            break;
        case MSG_MOVE:
            status = pthread_create(&gameThread, NULL, handleMove, (void *)msg);
            err(status, -1, "Move error.");
            pthread_detach(gameThread);
            break;
        default:
            break;
        }
    }
}

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        puts("Incorrect arguments. Usage: ./client NAME CONNECTION_TYPE ADDRESS [PORT (inet only)]");
        exit(EXIT_FAILURE);
    }
    if (!strcasecmp(argv[2], "local"))
    {
        isLocal = 1;
    }
    else if (!strcasecmp(argv[2], "inet"))
    {
        isLocal = 0;
    }
    else
    {
        puts("Incorrect mode. Available modes: INET LOCAL");
        exit(EXIT_FAILURE);
    }
    name = argv[1];
    address = argv[3];
    if (isLocal && argc != 5)
    {
        puts("Missing port number.");
        exit(EXIT_FAILURE);
    }
    else
    {
        port = atoi(argv[4]);
    }
    signal(SIGINT, disconnect);
    connectToServer();
    listenToServer();
}