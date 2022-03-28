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
        if ((pid = fork()) == 0)
        {
            printf("CHILD PID: %d\n", getpid());
            exit(0);
        }
    }
    return 0;
}