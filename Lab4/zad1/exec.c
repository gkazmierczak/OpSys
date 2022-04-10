#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

void sigusrHandler()
{
    printf("Received SIGUSR1 in process %d;\n", getpid());
    fflush(stdout);
    return;
}

void maskSigusr()
{
    sigset_t signalSet;
    sigset_t oldMask;
    sigemptyset(&signalSet);
    sigaddset(&signalSet, SIGUSR1);
    sigprocmask(SIG_SETMASK, &signalSet, &oldMask);
    return;
}

void checkPendingSignals()
{
    sigset_t signalSet;
    sigemptyset(&signalSet);
    sigpending(&signalSet);
    if (sigismember(&signalSet, SIGUSR1))
    {
        printf("SIGUSR1 in pending signals of process %d.\n", getpid());
    }
    else
    {
        printf("SIGUSR1 is NOT in pending signals of process %d.\n", getpid());
    }
    return;
}

void handleMode(char *mode)
{
    if (!strcasecmp(mode, "ignore"))
    {
        signal(SIGUSR1, SIG_IGN);
        raise(SIGUSR1);
        printf("SIGUSR1 ignored in parent process.\n");
    }
    if (!strcasecmp(mode, "handler"))
    {
        signal(SIGUSR1, sigusrHandler);
        raise(SIGUSR1);
    }
    if (!strcasecmp(mode, "mask"))
    {
        maskSigusr();
        raise(SIGUSR1);
        printf("SIGUSR1 masked in parent process.\n");
    }
    if (!strcasecmp(mode, "pending"))
    {
        maskSigusr();
        raise(SIGUSR1);
        checkPendingSignals();
    }
    execl("./child", "./child", mode, NULL);

    return;
}

int main(int argc, char **argv)
{
    if (argc == 2)
    {
        printf("MODE: %s \n", argv[1]);
        fflush(stdout);
        handleMode(argv[1]);
    }
    return 0;
}