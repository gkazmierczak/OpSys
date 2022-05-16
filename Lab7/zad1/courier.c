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

void getPizza()
{
    decrementSemaphore(semID, EMPTY_TABLE_SEMAPHORE);
    decrementSemaphore(semID, TABLE_SEMAPHORE);
    pizza = queuePop(table);
    getCurrentTime(timeBuffer);
    printf("DOSTAWCA: (%d %s) Pobieram pizze: %d. Liczba pizz na stole: %d\n", getpid(), timeBuffer, pizza, table->count);
    // Unlock table and indicate theres free space on it
    incrementSemaphore(semID, FULL_TABLE_SEMAPHORE);
    incrementSemaphore(semID, TABLE_SEMAPHORE);
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
    semID = getSemaphore();
    ovenMemID = getOvenMemory();
    oven = shmat(ovenMemID, NULL, 0);
    tableMemID = getTableMemory();
    table = shmat(tableMemID, NULL, 0);
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