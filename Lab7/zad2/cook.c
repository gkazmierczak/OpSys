#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include "common.h"

int semID;
int ovenMemID;
int tableMemID;
int pizzaType;
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

void preparePizza()
{
    pizzaType = rand() % 10;
    sleepRandom(1, 2);
    getCurrentTime(timeBuffer);
    printf("KUCHARZ: (%d %s) Przygotowuje pizze: %d\n", getpid(), timeBuffer, pizzaType);
}
void placeInOven()
{
    // Lock process until oven has at least one free space
    decrementSemaphore(fullOvenSem);

    // Lock process until cook can access the oven
    decrementSemaphore(ovenSem);
    addToQueue(pizzaType, oven);
    getCurrentTime(timeBuffer);
    printf("KUCHARZ: (%d %s) Dodałem pizze: %d. Liczba pizz w piecu: %d\n", getpid(), timeBuffer, pizzaType, oven->count);

    // Unlock oven
    incrementSemaphore(ovenSem);
}
void removeFromOven()
{
    // Lock process until cook can access the oven
    decrementSemaphore(ovenSem);

    getCurrentTime(timeBuffer);
    int pizza = queuePop(oven);
    printf("KUCHARZ: (%d %s) Wyjmuję pizze: %d. Liczba pizz w piecu: %d\n", getpid(), timeBuffer, pizza, oven->count);
    // Unlock oven and add a free space in oven
    incrementSemaphore(ovenSem);
    incrementSemaphore(fullOvenSem);

    // Lock process until table has at least one free slot and is accessible
    decrementSemaphore(fullTableSem);
    decrementSemaphore(tableSem);
    addToQueue(pizza, table);
    printf("KUCHARZ: (%d %s) Kładę pizze na stole: %d. Liczba pizz na stole: %d\n", getpid(), timeBuffer, pizza, table->count);
    // Unlock table and place additional pizza on it
    incrementSemaphore(emptyTableSem);
    incrementSemaphore(tableSem);
}
void getSemaphores()
{
    ovenSem = getSemaphore(OVEN_SEMAPHORE);
    fullOvenSem = getSemaphore(FULL_OVEN_SEMAPHORE);
    tableSem = getSemaphore(TABLE_SEMAPHORE);
    fullTableSem = getSemaphore(FULL_TABLE_SEMAPHORE);
    emptyTableSem = getSemaphore(EMPTY_TABLE_SEMAPHORE);
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
    srand(getpid());
    while (1)
    {
        preparePizza();
        placeInOven();
        sleepRandom(4, 5);
        removeFromOven();
    }
    sigHandler();
    return 0;
}