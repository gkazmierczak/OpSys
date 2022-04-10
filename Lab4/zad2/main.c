#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

int callIndex = 0;
int callDepth = 0;

void handleSiginfo(int sig, siginfo_t *sigInfo, void *context)
{
    puts("Signal info: ");
    printf("    Signal number: %d\n", sigInfo->si_signo);
    printf("    Signal code: %d\n", sigInfo->si_code);
    printf("    Errno value: %d\n", sigInfo->si_errno);
    printf("    Sender PID: %d\n", sigInfo->si_pid);
    printf("    Sender UID: %d\n", sigInfo->si_uid);
    printf("    Number of attempted system calls: %d\n", sigInfo->si_syscall);
    printf("    File descriptor: %d\n", sigInfo->si_fd);
    printf("    System time consumed: %lu\n", sigInfo->si_stime);
    fflush(stdout);
}

void handleNodefer(int sig, siginfo_t *sigInfo, void *context)
{
    puts("    Entering NODEFER flag handler");
    printf("    Call index: %d  , call depth: %d\n", callIndex, callDepth);
    callIndex++;
    callDepth++;
    if (callIndex < 10)
    {
        raise(SIGUSR1);
    }
    callDepth--;
    puts("Handler finished");
    fflush(stdout);
}
void handleResethand(int sig, siginfo_t *sigInfo, void *context)
{
    puts("    Entering RESETHAND flag handler");
    fflush(stdout);
}

int main(int argc, char **argv)
{
    puts("Testing SA_SIGINFO flag");
    static struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = handleSiginfo;
    sigaction(SIGUSR1, &act, NULL);
    sigaction(SIGUSR2, &act, NULL);
    raise(SIGUSR1);
    printf("\n\n");
    raise(SIGUSR2);

    puts("\n\nTesting SA_NODEFER flag");
    puts("SA_NODEFER should allow receiveg the signal from within the signal handler.");
    static struct sigaction act2;
    sigemptyset(&act2.sa_mask);
    act2.sa_flags = SA_NODEFER;
    act2.sa_sigaction = handleNodefer;
    sigaction(SIGUSR1, &act2, NULL);
    raise(SIGUSR1);

    puts("\n\nTesting SA_RESETHAND flag");
    puts("SA_RESETHAND should reset the signal handler to deafult action following its execution.");
    static struct sigaction act3;
    sigemptyset(&act3.sa_mask);
    act3.sa_flags = SA_RESETHAND;
    act3.sa_sigaction = handleResethand;
    sigaction(SIGUSR1, &act3, NULL);
    // Handler call expected
    raise(SIGUSR1);
    puts("Handler finished, expecting next raise to terminate the program\n");
    fflush(stdout);
    // Default action - program termination expected
    raise(SIGUSR1);
    puts("UNEXPECTED RESULT");
    fflush(stdout);
    return 0;
}