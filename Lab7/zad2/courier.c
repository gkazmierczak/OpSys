#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "common.h"

int semID;
int ovenMemID;
int tableMemID;
int pizza;
char *timeBuffer = NULL;
int pizzasInOven = 2;
int pizzasOnTable = 2;
struct CircleQueue *oven;
struct CircleQueue *table;
sem_t *ovenSem;
sem_t *fullOvenSem;
sem_t *tableSem;
sem_t *fullTableSem;
sem_t *emptyTableSem;

void sigHandler()
{
    if (timeBuffer != NULL)
    {
        free(timeBuffer);
    }
    munmap(oven, sizeof(struct CircleQueue));
    munmap(table, sizeof(struct CircleQueue));
    exit(0);
}
void getSemaphores()
{
    ovenSem = getSemaphore(OVEN_SEMAPHORE);
    fullOvenSem = getSemaphore(FULL_OVEN_SEMAPHORE);
    tableSem = getSemaphore(TABLE_SEMAPHORE);
    fullTableSem = getSemaphore(FULL_TABLE_SEMAPHORE);
    emptyTableSem = getSemaphore(EMPTY_TABLE_SEMAPHORE);
}
void getPizza()
{
    decrementSemaphore(emptyTableSem);
    decrementSemaphore(tableSem);
    pizza = queuePop(table);
    getCurrentTime(timeBuffer);
    printf("DOSTAWCA: (%d %s) Pobieram pizze: %d. Liczba pizz na stole: %d\n", getpid(), timeBuffer, pizza, table->count);
    // Unlock table and indicate theres free space on it
    incrementSemaphore(fullTableSem);
    incrementSemaphore(tableSem);
}
void deliverPizza()
{
    getCurrentTime(timeBuffer);
    printf("DOSTAWCA: (%d %s) Dostarczam pizze: %d\n", getpid(), timeBuffer, pizza);
}

int main()
{
    signal(SIGINT, sigHandler);
    timeBuffer = (char *)calloc(24, sizeof(char));
    getSemaphores();
    ovenMemID = getOvenMemory();
    oven = mmap(NULL, sizeof(struct CircleQueue), PROT_READ | PROT_WRITE, MAP_SHARED, ovenMemID, 0);
    tableMemID = getTableMemory();
    table = mmap(NULL, sizeof(struct CircleQueue), PROT_READ | PROT_WRITE, MAP_SHARED, tableMemID, 0);
    while (1)
    {
        getPizza();
        sleepRandom(4, 5);
        deliverPizza();
        sleepRandom(4, 5);
    }
    sigHandler();
    return 0;
}