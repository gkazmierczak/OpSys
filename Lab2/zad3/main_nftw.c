#define _XOPEN_SOURCE 500
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
#include <ftw.h>

struct FileTypeCounter
{
    long files;
    long dirs;
    long chardev;
    long blockdev;
    long fifo;
    long slink;
    long socket;
    long other;
};

struct FileTypeCounter *counter;
int maxdepth;

char *stModeToFiletype(mode_t st_mode, struct FileTypeCounter *counter)
{
    if (S_ISREG(st_mode))
    {
        counter->files += 1;
        return "file";
    }
    if (S_ISDIR(st_mode))
    {
        counter->dirs += 1;
        return "dir";
    }
    if (S_ISCHR(st_mode))
    {
        counter->chardev += 1;
        return "char dev";
    }
    if (S_ISBLK(st_mode))
    {
        counter->blockdev += 1;
        return "block dev";
    }
    if (S_ISFIFO(st_mode))
    {
        counter->fifo += 1;
        return "fifo";
    }
    if (S_ISLNK(st_mode))
    {
        counter->slink += 1;
        return "slink";
    }
    if (S_ISSOCK(st_mode))
    {
        counter->socket += 1;
        return "sock";
    }
    else
    {
        counter->other += 1;
        return "other";
    }
}

void printStat(char *path, struct stat *statBuffer)
{
    char accessTime[128];
    strftime(accessTime, 128, "%Y-%m-%d %H:%M:%S", localtime(&(statBuffer->st_atime)));
    char modTime[128];
    strftime(modTime, 128, "%Y-%m-%d %H:%M:%S", localtime(&(statBuffer->st_mtime)));
    printf("%s | Links: %ld | Type: %s | Size: %ld | Access time: %s | Modification time: %s\n", path, statBuffer->st_nlink, stModeToFiletype(statBuffer->st_mode, counter), statBuffer->st_size, accessTime, modTime);
    fflush(stdout);
}

int fileInfo(char *path, struct stat *statBuffer, int flags, struct FTW *ftwp)
{
    if (maxdepth != -1 && ftwp->level > maxdepth)
    {
        return 0;
    }
    printStat(path, statBuffer);
    return 0;
}
void crawlDirectory(char *dirname, int maxdepth)
{
    if (maxdepth == 0)
    {
        return;
    }
    nftw(dirname, (__nftw_func_t)fileInfo, 2, FTW_PHYS);
}

int main(int argc, char **argv)
{
    char *dirname = malloc(1024 * sizeof(char));
    maxdepth = -1;
    if (argc >= 2)
    {
        dirname = argv[1];
        if (argv[2] != NULL)
        {
            maxdepth = atoi(argv[2]);
        }
    }
    else
    {
        printf("Enter directory path: ");
        scanf("%s", dirname);
    }
    counter = (struct FileTypeCounter *)calloc(1, sizeof(struct FileTypeCounter));
    crawlDirectory(dirname, maxdepth);
    printf("Total files: %ld | Total dirs: %ld | Total char dev: %ld | Total block dev: %ld | Total fifo: %ld | Total sockets: %ld | Total links: %ld \n", counter->files, counter->dirs, counter->chardev, counter->blockdev, counter->fifo, counter->socket, counter->slink);
    fflush(stdout);
    return 0;
}
