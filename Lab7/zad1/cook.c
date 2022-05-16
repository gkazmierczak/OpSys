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

void sigHandler()
{
    // printf("\n PID: %d  - Closing... \n", getpid());
    if (timeBuffer != NULL)
    {
        free(timeBuffer);
    }
    shmdt(oven);
    shmdt(table);
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
    decrementSemaphore(semID, FULL_OVEN_SEMAPHORE);

    // Lock process until cook can access the oven
    decrementSemaphore(semID, OVEN_SEMAPHORE);
    addToQueue(pizzaType, oven);
    getCurrentTime(timeBuffer);
    printf("KUCHARZ: (%d %s) Dodałem pizze: %d. Liczba pizz w piecu: %d\n", getpid(), timeBuffer, pizzaType, oven->count);

    // Unlock oven
    incrementSemaphore(semID, OVEN_SEMAPHORE);
}
void removeFromOven()
{
    // Lock process until cook can access the oven
    decrementSemaphore(semID, OVEN_SEMAPHORE);

    getCurrentTime(timeBuffer);
    int pizza = queuePop(oven);
    printf("KUCHARZ: (%d %s) Wyjmuję pizze: %d. Liczba pizz w piecu: %d\n", getpid(), timeBuffer, pizza, oven->count);
    // Unlock oven and add a free space in oven
    incrementSemaphore(semID, OVEN_SEMAPHORE);
    incrementSemaphore(semID, FULL_OVEN_SEMAPHORE);

    // Lock process until table has at least one free slot and is accessible
    decrementSemaphore(semID, FULL_TABLE_SEMAPHORE);
    decrementSemaphore(semID, TABLE_SEMAPHORE);
    addToQueue(pizza, table);
    printf("KUCHARZ: (%d %s) Kładę pizze na stole: %d. Liczba pizz na stole: %d\n", getpid(), timeBuffer, pizza, table->count);
    // Unlock table and place additional pizza on it
    incrementSemaphore(semID, EMPTY_TABLE_SEMAPHORE);
    incrementSemaphore(semID, TABLE_SEMAPHORE);
}

int main()
{
    signal(SIGINT, sigHandler);
    timeBuffer = (char *)calloc(24, sizeof(char));
    semID = getSemaphore();
    ovenMemID = getOvenMemory();
    oven = shmat(ovenMemID, NULL, 0);
    tableMemID = getTableMemory();
    table = shmat(tableMemID, NULL, 0);
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