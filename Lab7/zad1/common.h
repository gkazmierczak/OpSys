#ifndef LAB7_COMMON_H
#define LAB7_COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>

#define PIZZERIA_SIZE 5
#define SEMAPHORE_COUNT 5
#define OVEN_SEMAPHORE 0
#define FULL_OVEN_SEMAPHORE 1
#define TABLE_SEMAPHORE 2
#define FULL_TABLE_SEMAPHORE 3
#define EMPTY_TABLE_SEMAPHORE 4

union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
    struct seminfo *__buf;
};

struct CircleQueue
{
    int array[PIZZERIA_SIZE];
    int count;
    int firstIdx;
    int lastIdx;
};

void sleepRandom(int from, int to);
int getSemaphore();
int getOvenMemory();
int getTableMemory();
void getCurrentTime(char *buf);
void addToQueue(int n, struct CircleQueue *q);
int queuePop(struct CircleQueue *q);
void incrementSemaphore(int semaphoreID, int semaphoreNum);
void decrementSemaphore(int semaphoreID, int semaphoreNum);

#endif // LAB7_COMMON_H