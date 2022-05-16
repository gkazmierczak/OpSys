#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include "common.h"

int semID;
int ovenMemID;
int tableMemID;
int *pids = NULL;
int cooks = 0;
int couriers = 0;

void sigHandler()
{
    printf("\n Closing... \n");
    if (pids != NULL)
    {
        for (int i = 0; i < couriers + cooks; i++)
        {
            if (pids[i] != 0)
            {
                kill(pids[i], SIGINT);
            }
        }
        free(pids);
    }
    semctl(semID, 0, IPC_RMID, NULL);
    shmctl(ovenMemID, IPC_RMID, NULL);
    shmctl(tableMemID, IPC_RMID, NULL);
    exit(0);
}

int createSemaphore()
{
    key_t semKey = ftok(getenv("HOME"), 0);
    int semID = semget(semKey, SEMAPHORE_COUNT, IPC_CREAT | 0666);
    if (semID < 0)
    {
        perror("Could not create semaphores");
    }
    union semun arg;
    arg.val = 1;
    if ((semctl(semID, 0, SETVAL, arg) == -1))
    {
        perror("Could not set OVEN semaphore value");
        exit(-1);
    };
    if ((semctl(semID, 2, SETVAL, arg) == -1))
    {
        perror("Could not set TABLE semaphore value");
        exit(-1);
    };
    arg.val = PIZZERIA_SIZE;
    if ((semctl(semID, 1, SETVAL, arg) == -1))
    {
        perror("Could not set FULL_OVEN semaphore value");
        exit(-1);
    };
    if ((semctl(semID, 3, SETVAL, arg) == -1))
    {
        perror("Could not set FULL_TABLE semaphore value");
        exit(-1);
    };
    arg.val = 0;
    if ((semctl(semID, 4, SETVAL, arg) == -1))
    {
        perror("Could not set EMPTY_TABLE semaphore value");
        exit(-1);
    };
    return semID;
}

int createSharedMemory(int proj_id)
{
    key_t shmKey = ftok(getenv("HOME"), proj_id);
    int shmID = shmget(shmKey, PIZZERIA_SIZE, IPC_CREAT | 0666);
    if (shmID < 0)
    {
        perror("Could not access shared memory");
        exit(-1);
    }
    return shmID;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Wrong number of arguments. Usage: ./zad1 COOKS COURIERS\n");
        exit(0);
    }
    cooks = atoi(argv[1]);
    couriers = atoi(argv[2]);
    signal(SIGINT, sigHandler);
    semID = createSemaphore();
    ovenMemID = createSharedMemory(1);
    tableMemID = createSharedMemory(2);
    pids = (int *)calloc(cooks + couriers, sizeof(int));
    for (int i = 0; i < cooks; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            execlp("./cook", "./cook", NULL);
        }
        pids[i] = pid;
        sleepRandom(1, 2);
    }
    for (int i = 0; i < couriers; i++)
    {
        pid_t pid = fork();
        if (pid == 0)
        {
            execlp("./courier", "./courier", NULL);
        }
        pids[cooks + i] = pid;
        sleepRandom(0, 1);
    }
    while (wait(0) != -1)
        ;
    sigHandler();
    return 0;
}