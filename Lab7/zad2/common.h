#ifndef LAB7_COMMON_H
#define LAB7_COMMON_H

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#define PIZZERIA_SIZE 5
#define SEMAPHORE_COUNT 5
#define OVEN_SEMAPHORE "/OVEN_SEM"
#define FULL_OVEN_SEMAPHORE "/FULL_OVEN_SEM"
#define TABLE_SEMAPHORE "/TABLE_SEM"
#define FULL_TABLE_SEMAPHORE "/FULL_TABLE_SEM"
#define EMPTY_TABLE_SEMAPHORE "/EMPTY_TABLE_SEM"
#define TABLE_MEM "/TABLE_MEM"
#define OVEN_MEM "/OVEN_MEM"

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
sem_t *getSemaphore(char *name);
int getOvenMemory();
int getTableMemory();
void getCurrentTime(char *buf);
void addToQueue(int n, struct CircleQueue *q);
int queuePop(struct CircleQueue *q);
void incrementSemaphore(sem_t *sem);
void decrementSemaphore(sem_t *sem);

#endif // LAB7_COMMON_H