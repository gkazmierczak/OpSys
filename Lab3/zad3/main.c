#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>

void checkFile(char *filepath, int pid, char *pattern)
{
    char ELF[5];
    ELF[0] = 0x7f;
    ELF[1] = 'E';
    ELF[2] = 'L';
    ELF[3] = 'F';
    ELF[4] = '\0';
    char *test = calloc(8, sizeof(char));
    FILE *fp = fopen(filepath, "r");
    char *lineBuffer = NULL;
    fread(test, 1, 4, fp);
    test[4] = '\0';
    if (strcmp(test, ELF))
    {
        fseek(fp, 0, SEEK_SET);
        size_t lineLen = 0;
        ssize_t read;
        while ((read = getline(&lineBuffer, &lineLen, fp)) != -1)
        {
            if (strstr(lineBuffer, pattern) != NULL)
            {
                printf("PID:  %d  |  PATH:  %s  \n", pid, filepath);
                fflush(stdout);
                break;
            }
        }
    }
    fclose(fp);
    free(test);
    free(lineBuffer);
    return;
}

void crawlDirectory(char *dirname, int maxdepth, char *pattern)
{
    if (maxdepth == 0)
    {
        exit(0);
    }
    DIR *currentDir = opendir(dirname);
    if (currentDir == NULL)
    {
        perror("Error opening directory: ");
        fflush(stdout);
        exit(-1);
    }
    struct dirent *currentFile = readdir(currentDir);
    char *newPath = (char *)calloc(2048, sizeof(char));
    struct stat *statBuffer = (struct stat *)calloc(1, sizeof(struct stat));
    int pid;
    int childrenCount = 0;
    while (currentFile != NULL)
    {
        strcpy(newPath, dirname);
        strcat(newPath, "/");
        strcat(newPath, currentFile->d_name);
        if (lstat(newPath, statBuffer) < 0)
        {
            printf("Unable to lstat file %s :", newPath);
            perror("");
            fflush(stdout);
            free(statBuffer);
            free(newPath);
            closedir(currentDir);
            exit(-1);
        }
        if (S_ISDIR(statBuffer->st_mode))
        {
            if (strcmp(currentFile->d_name, ".") && strcmp(currentFile->d_name, ".."))
            {
                if ((pid = fork()) == 0)
                {
                    crawlDirectory(newPath, maxdepth - 1, pattern);
                    exit(0);
                }
                else
                {
                    childrenCount++;
                }
            }
        }
        else if (S_ISREG(statBuffer->st_mode))
        {
            checkFile(newPath, getpid(), pattern);
        }
        currentFile = readdir(currentDir);
    }
    closedir(currentDir);
    free(statBuffer);
    free(newPath);
    for (int i = 0; i < childrenCount; i++)
    {
        wait(0);
    }
    return;
}

int main(int argc, char **argv)
{
    char *dirname = NULL;
    char *pattern = NULL;
    int maxdepth = -1;
    if (argc >= 4)
    {

        dirname = argv[1];
        pattern = argv[2];
        maxdepth = atoi(argv[3]);
    }
    else if (argc == 2)
    {
        dirname = argv[1];
        pattern = malloc(1024 * sizeof(char));
        printf("Enter pattern: ");
        scanf("%s", pattern);
    }
    else if (argc == 1)
    {
        dirname = malloc(1024 * sizeof(char));
        pattern = malloc(1024 * sizeof(char));
        printf("Enter directory path: ");
        scanf("%s", dirname);
        printf("Enter pattern: ");
        scanf("%s", pattern);
    }
    else
    {
        dirname = argv[1];
        pattern = argv[2];
    }
    DIR *dp;
    if ((dp = opendir(dirname)) != NULL)
    {
        closedir(dp);
        struct stat *statBuffer = (struct stat *)calloc(1, sizeof(struct stat));
        lstat(dirname, statBuffer);
        free(statBuffer);
        crawlDirectory(dirname, maxdepth, pattern);
    }
    else
    {
        perror("Cannot open directory ");
        fflush(stdout);
    }
    if (argc <= 2)
        free(pattern);
    if (argc == 1)
        free(dirname);

    return 0;
}
