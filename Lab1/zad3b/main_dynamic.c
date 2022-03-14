#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <dlfcn.h>

typedef struct BlockArray
{
    int size;
    char **array;
} BlockArray;

static clock_t startTime, endTime;
static struct tms st_cpu, en_cpu;
FILE *reportsFile;
void *dlhandle;

void startTimer(void)
{
    startTime = times(&st_cpu);
}

void endTimer(void)
{
    endTime = times(&en_cpu);
}

void printReportHeader(void)
{
    fprintf(reportsFile, "\nReal time [s] | System time [s] | User time [s] | Test description  \n");
}

void saveTimer(char *name)
{
    int clkTicks = sysconf(_SC_CLK_TCK);
    printf("%ld %ld %ld %ld %ld %ld", endTime, startTime, en_cpu.tms_cstime, st_cpu.tms_cstime, en_cpu.tms_cutime, st_cpu.tms_cutime);
    double realTime = (double)(endTime - startTime) / clkTicks;
    double systemTime = (double)(en_cpu.tms_stime - st_cpu.tms_stime) / clkTicks;
    double userTime = (double)(en_cpu.tms_utime - st_cpu.tms_utime) / clkTicks;
    fprintf(reportsFile, "%13f | %15f | %13f | %s \n", realTime, systemTime, userTime, name);
}

void handleTimerEnd(char *testName)
{
    endTimer();
    if (startTime != 0)
    {
        saveTimer(testName);
    }
    return;
}

char *generateRandomString(int size)
{
    if (size < 1)
    {
        printf("-i: Incorrect SIZE argument\n");
        return NULL;
    }
    char *base = "0123456789QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm _.";
    char *result = (char *)calloc(size, sizeof(char));
    for (int i = 0; i < size - 1; i++)
    {
        result[i] = base[rand() % strlen(base)];
    }
    result[size - 1] = '\0';
    return result;
}

void insertNBlocks(BlockArray *blockArray, int count, int startIndex, int blockSize)
{
    if (count > blockArray->size)
    {
        printf("Incorrect argument: Cannot insert %d blocks into %d-sized BlockArray.\n", count, blockArray->size);
        return;
    }
    int (*insertIntoArrayAt)(BlockArray *, int, char *) = dlsym(dlhandle, "insertIntoArrayAt");
    if (dlerror() != NULL)
    {
        printf("Error loading library symbol");
        return;
    }
    char *data = generateRandomString(blockSize);
    if (data == NULL)
        return;
    for (int i = 0; i < count; i++)
    {
        insertIntoArrayAt(blockArray, i + startIndex, data);
    }
    free(data);
}

void deleteNBlocks(BlockArray *blockArray, int count, int startIndex)
{
    if (count > blockArray->size)
    {
        printf("Incorrect argument: Cannot delete %d blocks from %d-sized BlockArray.\n", count, blockArray->size);
        return;
    }
    int (*deleteBlock)(BlockArray *, int) = dlsym(dlhandle, "deleteBlock");
    if (dlerror() != NULL)
    {
        printf("Error loading library symbol");
        return;
    }
    for (int i = 0; i < count; i++)
    {
        deleteBlock(blockArray, i + startIndex);
        if (i + startIndex >= blockArray->size)
        {
            return;
        }
    }
}

void alternateAddFree(BlockArray *blockArray, int times, int count, int idx, int size)
{
    int (*insertIntoArrayAt)(BlockArray *, int, char *) = dlsym(dlhandle, "insertIntoArrayAt");
    int (*deleteBlock)(BlockArray *, int) = dlsym(dlhandle, "deleteBlock");
    if (dlerror() != NULL)
    {
        printf("Error loading library symbol");
        return;
    }
    for (int i = 0; i < times; i++)
    {
        insertNBlocks(blockArray, count, idx, size);
        deleteNBlocks(blockArray, count, idx);
    }
}
/*
    Program options:
        -c SIZE - creates a BlockArray of SIZE and sets it as current BlockArray
        -w FILENAMES -  performs wc-like count on specified files and stores the result in a tempfile
        -r INDEX - removes block at given index
        -s - stores contents of tempfile in first free block of BlockArray, outputs index
        -g INDEX - outputs content of block at specified INDEX
        -t - starts timing clock
        -e "TEST DESC" - stops timing clock, outputs measured time to file  with TEST DESC description if -t was used before
        -i COUNT INDEX SIZE - generates random string with SIZE chars and tries to insert it into BlockArray at COUNT indexes starting from INDEX
        -d COUNT INDEX - tries to remove COUNT blocks under indexes starting from INDEX, empty indexes are counted as deleted, stops at array end
*/
int main(int argc, char **argv)
{
    dlhandle = dlopen("./library.so", RTLD_LAZY);
    if (!dlhandle)
    {
        printf("Failed to open library, exiting.");
        return -1;
    }
    BlockArray *(*initBlockArray)(int) = dlsym(dlhandle, "initBlockArray");
    void (*deleteBlockArray)(BlockArray *) = dlsym(dlhandle, "deleteBlockArray");
    if (dlerror() != NULL)
    {
        printf("Error loading library symbols.");
        return -1;
    }
    int opt, firstFilenameIndex, lastFilenameIndex, count, idx;
    BlockArray *blockArray = NULL;
    reportsFile = fopen("results3.txt", "a+");
    while ((opt = getopt(argc, argv, "acdeghirstw")) != -1)
    {
        switch (opt)
        {
        case 'c':
            if (optind < argc)
            {
                if (blockArray != NULL)
                {
                    deleteBlockArray(blockArray);
                }
                blockArray = initBlockArray(atoi(argv[optind++]));
            }
            else
            {
                printf("Missing argument, usage: -c SIZE\n");
                return -1;
            }
            break;
        case 'w':
            firstFilenameIndex = optind;
            lastFilenameIndex = 0;
            while (optind < argc)
            {
                if (argv[optind][0] == '-')
                {
                    break;
                }
                lastFilenameIndex = ++optind;
            }
            if (lastFilenameIndex > firstFilenameIndex)
            {
                count = lastFilenameIndex - firstFilenameIndex;
                int (*countFiles)(int, int, char **) = dlsym(dlhandle, "countFiles");
                if (dlerror() != NULL)
                {
                    printf("Error loading library symbol");
                    return -1;
                }
                countFiles(count, firstFilenameIndex, argv);
            }
            else
            {
                printf("Missing argument, usage: -w FILENAME\n");
                return -1;
            }
            break;
        case 'r':
            if (optind < argc)
            {
                int (*deleteBlock)(BlockArray *, int) = dlsym(dlhandle, "deleteBlock");
                if (dlerror() != NULL)
                {
                    printf("Error loading library symbol");
                    return -1;
                }
                deleteBlock(blockArray, atoi(argv[optind++]));
            }
            else
            {
                printf("Missing argument, usage: -r INDEX\n");
                return -1;
            }
            break;
        case 's':
            idx = -1;
            int (*storeTempfile)(BlockArray *) = dlsym(dlhandle, "storeTempfile");
            if (dlerror() != NULL)
            {
                printf("Error loading library symbol");
                return -1;
            }
            idx = storeTempfile(blockArray);
            if (idx >= 0)
            {
                printf("Tempfile content stored at index: %d\n", idx);
            }
            else
            {
                printf("Error storing tempfile contents");
                return -1;
            }
            break;
        case 'g':
            if (optind < argc)
            {
                char *(*getBlock)(BlockArray *, int) = dlsym(dlhandle, "getBlock");
                if (dlerror() != NULL)
                {
                    printf("Error loading library symbol");
                    return -1;
                }
                printf("Block content: %s\n", getBlock(blockArray, atoi(argv[optind++])));
            }
            break;
        case 't':
            startTimer();
            break;
        case 'h':
            printReportHeader();
            break;
        case 'e':
            if (optind < argc)
            {
                handleTimerEnd(argv[optind++]);
            }
            else
            {
                handleTimerEnd("");
            }
            break;
        case 'i':
            if (optind + 2 < argc)
            {
                insertNBlocks(blockArray, atoi(argv[optind]), atoi(argv[optind + 1]), atoi(argv[optind + 2]));
                optind += 3;
            }
            break;
        case 'd':
            if (optind + 1 < argc)
            {
                deleteNBlocks(blockArray, atoi(argv[optind]), atoi(argv[optind + 1]));
                optind += 2;
            }
            break;
        case 'a':
            if (optind + 3 < argc)
            {
                alternateAddFree(blockArray, atoi(argv[optind]), atoi(argv[optind + 1]), atoi(argv[optind + 2]), atoi(argv[optind + 3]));
                optind += 4;
            }
            break;
        default:
            printf("Unrecognized option: -%c %d\n", optopt, optind);
            break;
        }
    }
    fclose(reportsFile);
    deleteBlockArray(blockArray);
    dlclose(dlhandle);
    return 0;
}