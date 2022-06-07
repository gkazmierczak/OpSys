#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#define REINDEER_TOTAL 9
#define ELVES_TOTAL 10
#define ELVES_REQ 3

pthread_mutex_t santaMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t elfMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t elvesWaitingMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reindeerMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t reindeerWaitingMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t santaCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t reindeerWaitingCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t elvesWaitingCond = PTHREAD_COND_INITIALIZER;
pthread_cond_t elfFixCond = PTHREAD_COND_INITIALIZER;

int elfQueue[3];
pthread_t santaThread;
pthread_t elfThreads[ELVES_TOTAL];
pthread_t reindeerThreads[REINDEER_TOTAL];
int reindeerBack = 0;
int elvesWaiting = 0;

void sleepRandom(int from, int to)
{
    int part = (int)(((to - from) * ((double)rand() / (double)RAND_MAX)) * 1000000);
    usleep(from * 1000000 + part);
};

void *reindeer(void *arg)
{

    int reindeerId = *((int *)arg);
    while (1)
    {
        sleepRandom(5, 10);
        pthread_mutex_lock(&reindeerMutex);
        printf("Renifer: czeka %d reniferów na Mikołaja, %d\n", ++reindeerBack, reindeerId);
        if (reindeerBack == REINDEER_TOTAL)
        {
            printf("Renifer: wybudzam Mikołaja, %d\n", reindeerId);
            pthread_cond_signal(&santaCond);
            pthread_cond_broadcast(&reindeerWaitingCond);
        }
        while (reindeerBack > 0)
        {
            pthread_cond_wait(&reindeerWaitingCond, &reindeerMutex);
        }
        pthread_mutex_unlock(&reindeerMutex);
    }
    return NULL;
}
void *elf(void *arg)
{

    int elfID = *((int *)arg);
    while (1)
    {
        sleepRandom(2, 5);
        pthread_mutex_lock(&elvesWaitingMutex);
        if (elvesWaiting == ELVES_REQ)
        {
            printf("Elf: czekam na powrót elfów, %d\n", elfID);
            while (elvesWaiting == ELVES_REQ)
            {
                pthread_cond_wait(&elvesWaitingCond, &elvesWaitingMutex);
            }
        }
        pthread_mutex_unlock(&elvesWaitingMutex);
        pthread_mutex_lock(&elfMutex);
        int i = elvesWaiting;
        if (elvesWaiting < ELVES_REQ)
        {
            elfQueue[i] = elfID;
            printf("Elf: czeka %d elfów na Mikołaja, %d\n", ++elvesWaiting, elfID);
        }
        if (elvesWaiting == ELVES_REQ)
        {
            printf("Elf: wybudzam Mikołaja, %d\n", elfID);
            pthread_cond_signal(&santaCond);
            pthread_cond_broadcast(&elvesWaitingCond);
        }
        if (elfQueue[i] == elfID)
        {
            pthread_cond_wait(&elfFixCond, &elfMutex);
            printf("Elf: Mikołaj rozwiązuje problem, %d\n", elfID);
        }
        pthread_mutex_unlock(&elfMutex);
    }
    return NULL;
}
void *santa(void *arg)
{
    int giftsDelivered = 0;
    while (1)
    {
        pthread_mutex_lock(&santaMutex);
        while (reindeerBack < REINDEER_TOTAL && elvesWaiting < ELVES_REQ)
        {
            puts("Mikołaj: zasypiam");
            pthread_cond_wait(&santaCond, &santaMutex);
            puts("Mikołaj: budzę się");
        }
        if (reindeerBack == REINDEER_TOTAL)
        {
            puts("Mikołaj: dostarczam zabawki");
            sleepRandom(2, 4);
            giftsDelivered++;
            reindeerBack = 0;
            pthread_cond_broadcast(&reindeerWaitingCond);
            puts("Mikołaj: dostarczyłem zabawki");

            if (giftsDelivered == 3)
            {
                pthread_exit(NULL);
            }
        }
        if (elvesWaiting == ELVES_REQ)
        {
            sleepRandom(1, 2);
            printf("Mikołaj: rozwiązuje problemy elfów %d %d %d\n", elfQueue[0], elfQueue[1], elfQueue[2]);
            elvesWaiting = 0;
            pthread_cond_broadcast(&elfFixCond);
            pthread_cond_broadcast(&elvesWaitingCond);
            printf("Mikołaj: rozwiązałem problemy elfów\n");
        }
        pthread_mutex_unlock(&santaMutex);
    }
    return NULL;
}

void cleanup()
{
    for (int i = 0; i < REINDEER_TOTAL; i++)
    {
        pthread_cancel(reindeerThreads[i]);
        pthread_detach(reindeerThreads[i]);
    }
    for (int i = 0; i < ELVES_TOTAL; i++)
    {
        pthread_cancel(elfThreads[i]);
        pthread_detach(elfThreads[i]);
    }
    pthread_mutex_destroy(&santaMutex);
    pthread_mutex_destroy(&elfMutex);
    pthread_mutex_destroy(&reindeerMutex);
    pthread_mutex_destroy(&elvesWaitingMutex);
    pthread_mutex_destroy(&reindeerWaitingMutex);
    pthread_cond_destroy(&santaCond);
    pthread_cond_destroy(&elfFixCond);
    pthread_cond_destroy(&elvesWaitingCond);
    pthread_cond_destroy(&reindeerWaitingCond);
}

int main(int argc, char **argv)
{
    atexit(cleanup);
    int reindeerIDs[REINDEER_TOTAL];
    int elfIDs[ELVES_TOTAL];
    pthread_create(&santaThread, NULL, santa, NULL);
    for (int i = 0; i < REINDEER_TOTAL; i++)
    {
        reindeerIDs[i] = i;
        pthread_create(&reindeerThreads[i], NULL, reindeer, &reindeerIDs[i]);
    }
    for (int i = 0; i < ELVES_TOTAL; i++)
    {
        elfIDs[i] = i;
        pthread_create(&elfThreads[i], NULL, elf, &elfIDs[i]);
    }
    pthread_join(santaThread, NULL);
    exit(0);
}