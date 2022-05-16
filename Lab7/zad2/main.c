#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include "common.h"

int ovenMemID;
int tableMemID;
struct CircleQueue *oven;
struct CircleQueue *table;
int *pids = NULL;
int cooks = 0;
int couriers = 0;

void unlinkSemaphores()
{
    sem_unlink(OVEN_SEMAPHORE);
    sem_unlink(TABLE_SEMAPHORE);
    sem_unlink(FULL_OVEN_SEMAPHORE);
    sem_unlink(FULL_TABLE_SEMAPHORE);
    sem_unlink(EMPTY_TABLE_SEMAPHORE);
}

void unlinkSharedMemory()
{
    munmap(oven, sizeof(struct CircleQueue));
    munmap(table, sizeof(struct CircleQueue));
    shm_unlink(TABLE_MEM);
    shm_unlink(OVEN_MEM);
}

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
    unlinkSemaphores();
    unlinkSharedMemory();
    exit(0);
}

void createSemaphores()
{
    sem_t *ovenSem = sem_open(OVEN_SEMAPHORE, O_CREAT, 0666, 1);
    if (ovenSem == SEM_FAILED)
    {
        perror("Could not create oven semaphore");
    }
    sem_t *tableSem = sem_open(TABLE_SEMAPHORE, O_CREAT, 0666, 1);
    if (tableSem == SEM_FAILED)
    {
        perror("Could not create table semaphore");
    }
    sem_t *fullOvenSem = sem_open(FULL_OVEN_SEMAPHORE, O_CREAT, 0666, PIZZERIA_SIZE);
    if (fullOvenSem == SEM_FAILED)
    {
        perror("Could not create full oven semaphore");
    }
    sem_t *fullTableSem = sem_open(FULL_TABLE_SEMAPHORE, O_CREAT, 0666, PIZZERIA_SIZE);
    if (fullTableSem == SEM_FAILED)
    {
        perror("Could not create full table semaphore");
    }
    sem_t *emptyTablenSem = sem_open(EMPTY_TABLE_SEMAPHORE, O_CREAT, 0666, 0);
    if (emptyTablenSem == SEM_FAILED)
    {
        perror("Could not create empty table semaphore");
    }
}

void createSharedMemory()
{
    ovenMemID = shm_open(OVEN_MEM, O_CREAT | O_RDWR, 0666);
    tableMemID = shm_open(TABLE_MEM, O_CREAT | O_RDWR, 0666);
    if (ovenMemID == -1 || tableMemID == -1)
    {
        perror("Could not access shared memory");
        exit(-1);
    }
    ftruncate(ovenMemID, sizeof(struct CircleQueue));
    ftruncate(tableMemID, sizeof(struct CircleQueue));
    oven = mmap(NULL, sizeof(struct CircleQueue), PROT_READ | PROT_WRITE, MAP_SHARED, ovenMemID, 0);
    table = mmap(NULL, sizeof(struct CircleQueue), PROT_READ | PROT_WRITE, MAP_SHARED, tableMemID, 0);
    if (oven == (void *)-1 || table == (void *)-1)
    {
        perror("Could not attach shared memeory segment");
        exit(-1);
    }
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
    createSemaphores();
    createSharedMemory();
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