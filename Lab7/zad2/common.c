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

sem_t *getSemaphore(char *name)
{
    sem_t *sem = sem_open(name, O_RDWR);
    if (sem == SEM_FAILED)
    {
        perror("Could not get semaphore");
        exit(-1);
    }
    return sem;
};

int getOvenMemory()
{
    int shmID = shm_open(OVEN_MEM, O_RDWR, 0666);
    if (shmID == -1)
    {
        perror("Could not access shared memory");
        exit(-1);
    }
    return shmID;
}

int getTableMemory()
{
    int shmID = shm_open(TABLE_MEM, O_RDWR, 0666);
    if (shmID == -1)
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

void incrementSemaphore(sem_t *sem)
{
    if (sem_post(sem) == -1)
    {
        perror("Could not increment semaphore");
        exit(-1);
    }
}
void decrementSemaphore(sem_t *sem)
{
    if (sem_wait(sem) == -1)
    {
        perror("Could not decrement semaphore");
        exit(-1);
    }
}