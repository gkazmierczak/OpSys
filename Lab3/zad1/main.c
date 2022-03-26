#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int childProcessCount = atoi(argv[1]);
    int pid = 0;
    for (int i = 0; i < childProcessCount; i++)
    {
        if (pid == 0)
        {
            pid = fork();
        }
    }
    printf("PID: %d\n", getpid());
    return 0;
}