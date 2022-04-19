#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void printSortedMail(char *mode)
{
    FILE *pipe;
    /*
        Open pipe to read mail list
            tail -n +2 skips two first lines
            head -n -2 skips last two lines
            sort -k 3 sorts by sender mail
        default sorting order seems to be by date
    */
    if (!strcasecmp(mode, "nadawca"))
    {
        pipe = popen("echo | mail | tail -n +2 | head -n -2 | sort -k 3", "r");
    }
    else if (!strcasecmp(mode, "data"))
    {
        pipe = popen("echo | mail | tail -n +2 | head -n -2", "r");
    }
    else
    {
        fprintf(stderr, "Incorrect sorting mode.\n");
        exit(-1);
    }
    if (pipe == NULL)
    {
        fprintf(stderr, "Error opening pipe.\n");
        exit(-1);
    }
    char line[1024];
    while (fgets(line, 1024, pipe) != NULL)
    {
        printf("%s", line);
        fflush(stdout);
    }
    pclose(pipe);
    return;
}
void sendMail(char *mail, char *subject, char *body)
{
    FILE *pipe;
    char *command = malloc(sizeof mail + sizeof subject + sizeof body + sizeof(char) * 32);
    sprintf(command, "echo %s | mail -s %s %s", body, subject, mail);
    pipe = popen(command, "r");
    if (pipe == NULL)
    {
        puts("Error opening pipe.");
        exit(-1);
    }
    pclose(pipe);
    free(command);
    return;
}

int main(int argc, char **argv)
{
    if (argc == 2)
    {
        printSortedMail(argv[1]);
    }
    else if (argc == 4)
    {
        sendMail(argv[1], argv[2], argv[3]);
    }
    else
    {
        fprintf(stderr, "Incorrect number of arguments!\n");
        fflush(stderr);
        return -1;
    }
    return 0;
}