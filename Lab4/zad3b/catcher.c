#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

volatile sig_atomic_t signalsReceived = 0;
int mode;
volatile pid_t senderPid;

void sig1Handler(int sig, siginfo_t *sigInfo, void *context)
{
    signalsReceived++;
    printf("Caught %d signals so far.\n", signalsReceived);

    if (!senderPid)
    {
        senderPid = sigInfo->si_pid;
    }

    if (mode == 0)
    {
        kill(senderPid, SIGUSR1);
    }
    else if (mode == 1)
    {
        union sigval val;
        val.sival_int = signalsReceived;
        sigqueue(senderPid, SIGUSR1, val);
    }
    else
    {
        kill(senderPid, SIGRTMIN);
    }
    printf("SENT SIGNAL TO %d\n", senderPid);
}

void sig2Handler(int sig, siginfo_t *sigInfo, void *context)
{
    puts("------------- FINAL SIGNAL RECEIVED -------------");
    printf("Received SIGUSR2, - all %d received SIGUSR1's should have been sent back to sender.\n", signalsReceived);
    pid_t senderPid = sigInfo->si_pid;
    if (mode == 0)
    {
        kill(senderPid, SIGUSR2);
    }
    else if (mode == 1)
    {
        union sigval val;
        val.sival_int = 0;
        sigqueue(senderPid, SIGUSR2, val);
    }
    else
    {
        kill(senderPid, SIGRTMIN + 1);
    }
    exit(0);
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        puts("Missing MODE argument. Available modes: kill | sigqueue | sigrt");
        exit(-1);
    }
    printf("PID: %d\n", getpid());
    struct sigaction sigact1 = {0};
    sigact1.sa_flags = SA_SIGINFO;
    sigact1.sa_sigaction = sig1Handler;
    struct sigaction sigact2 = {0};
    sigact2.sa_flags = SA_SIGINFO;
    sigact2.sa_sigaction = sig2Handler;
    sigset_t mask;
    sigfillset(&mask);
    if (!strcasecmp("kill", argv[1]))
    {
        mode = 0;
        sigaction(SIGUSR1, &sigact1, NULL);
        sigaction(SIGUSR2, &sigact2, NULL);
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
    }
    else if (!strcasecmp("sigqueue", argv[1]))
    {
        mode = 1;
        sigaction(SIGUSR1, &sigact1, NULL);
        sigaction(SIGUSR2, &sigact2, NULL);
        sigdelset(&mask, SIGUSR1);
        sigdelset(&mask, SIGUSR2);
    }
    else if (!strcasecmp("sigrt", argv[1]))
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
    signalsReceived = 0;
    while (1)
    {
        sigsuspend(&mask);
    }
    return 0;
}