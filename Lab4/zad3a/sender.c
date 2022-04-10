#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

volatile sig_atomic_t signalsReceived = 0;
int mode, signalCount;
void sig1Handler(int sig, siginfo_t *sigInfo, void *context)
{
    signalsReceived++;
    if (mode == 1)
    {
        printf("Received SIGUSR1 sent back by catcher [%d]\n", sigInfo->si_value.sival_int);
    }
}
void sig2Handler(int sig)
{
    puts("------------- FINAL SIGNAL RECEIVED -------------");
    printf("Received %d/%d signals back from catcher.\n", signalsReceived, signalCount);
    exit(0);
}

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        puts("Missing arguments. Usage: sender PID COUNT MODE\n Available modes: kill | sigqueue | sigrt");
        exit(-1);
    }
    int targetPid = atoi(argv[1]);
    signalCount = atoi(argv[2]);
    struct sigaction sigact1 = {0};
    sigact1.sa_flags = SA_SIGINFO;
    sigact1.sa_sigaction = sig1Handler;
    struct sigaction sigact2 = {0};
    sigact2.sa_handler = sig2Handler;
    sigset_t mask;
    sigfillset(&mask);
    if (!strcasecmp("kill", argv[3]))
    {
        mode = 0;
        sigaction(SIGUSR1, &sigact1, NULL);
        sigaction(SIGUSR2, &sigact2, NULL);
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
    }
    else if (!strcasecmp("sigqueue", argv[3]))
    {
        mode = 1;
        sigaction(SIGUSR1, &sigact1, NULL);
        sigaction(SIGUSR2, &sigact2, NULL);
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
    }
    else if (!strcasecmp("sigrt", argv[3]))
    {
        mode = 2;
        sigaction(SIGRTMIN, &sigact1, NULL);
        sigaction(SIGRTMIN + 1, &sigact2, NULL);
        sigdelset(&mask, SIGRTMIN);
        sigdelset(&mask, SIGRTMIN + 1);
    }
    else
    {
        puts("Wrong mode, exiting...");
        exit(-1);
    }
    if (mode == 0)
    {
        for (int i = 0; i < signalCount; i++)
        {
            kill(targetPid, SIGUSR1);
        }
        printf("%d signals sent to catcher.\n", signalCount);
        kill(targetPid, SIGUSR2);
    }
    else if (mode == 1)
    {
        union sigval val;
        for (int i = 0; i < signalCount; i++)
        {
            val.sival_int = i;
            sigqueue(targetPid, SIGUSR1, val);
        }
        printf("%d signals sent to catcher.\n", signalCount);
        val.sival_int = 0;
        sigqueue(targetPid, SIGUSR2, val);
    }
    else
    {
        for (int i = 0; i < signalCount; i++)
        {
            kill(targetPid, SIGRTMIN);
        }
        printf("%d signals sent to catcher.\n", signalCount);

        kill(targetPid, SIGRTMIN + 1);
    }
    while (1)
    {
        sigsuspend(&mask);
    }
    return 0;
}