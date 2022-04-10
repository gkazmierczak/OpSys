#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

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

int main(int argc, char **argv)
{
    if (strcasecmp(argv[1], "pending"))
    {
        raise(SIGUSR1);
    }
    else
    {
        checkPendingSignals();
    }
    return 0;
}