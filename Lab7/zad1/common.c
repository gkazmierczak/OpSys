#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "common.h"

void sleepRandom(int from, int to)
{
    int part = (int)(((to - from) * ((double)rand() / (double)RAND_MAX)) * 1000000);
    usleep(from * 1000000 + part);
};

int getSemaphore()
{
    key_t semKey = ftok(getenv("HOME"), 0);
    int semID = semget(semKey, 0, 0);
    if (semID < 0)
    {
        perror("Could not get semaphore");
        exit(-1);
    }
    return semID;
};

int getOvenMemory()
{
    key_t shmKey = ftok(getenv("HOME"), 1);
    int shmID = shmget(shmKey, 0, 0);
    if (shmID < 0)
    {
        perror("Could not access shared memory");
        exit(-1);
    }
    return shmID;
}

int getTableMemory()
{
    key_t shmKey = ftok(getenv("HOME"), 2);
    int shmID = shmget(shmKey, 0, 0);
    if (shmID < 0)
    {
        perror("Could not access shared memory");
        exit(-1);
    }
    return shmID;
}

void getCurrentTime(char *buf)
{
    struct timeval currTime;
    gettimeofday(&currTime, NULL);
    int ms = currTime.tv_usec / 1000;
    char *timeTmp = (char *)calloc(21, sizeof(char));
    strftime(timeTmp, 80, "%Y-%m-%d %H:%M:%S", localtime(&currTime.tv_sec));
    sprintf(buf, "%s:%03d", timeTmp, ms);
}
void addToQueue(int elem, struct CircleQueue *q)
{
    q->array[q->lastIdx] = elem;
    q->lastIdx = (q->lastIdx + 1) % PIZZERIA_SIZE;
    q->count++;
}
int queuePop(struct CircleQueue *q)
{
    int elem = q->array[q->firstIdx];
    q->firstIdx = (q->firstIdx + 1) % PIZZERIA_SIZE;
    q->count--;
    return elem;
}

void incrementSemaphore(int semaphoreID, int semaphoreNum)
{
    struct sembuf buffer = {semaphoreNum, 1, 0};
    if (semop(semaphoreID, &buffer, 1) == -1)
    {
        perror("Could not increment semaphore");
        exit(-1);
    }
}
void decrementSemaphore(int semaphoreID, int semaphoreNum)
{
    struct sembuf buffer = {semaphoreNum, -1, 0};
    if (semop(semaphoreID, &buffer, 1) == -1)
    {
        perror("Could not decrement semaphore");
        exit(-1);
    }
}