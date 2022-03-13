#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>

static clock_t startTime, endTime;
static struct tms st_cpu, en_cpu;
FILE *reportsFile;

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
    fprintf(reportsFile, "Real time [s] | System time [s] | User time [s] | Test description  \n");
}

void saveTimer(char *name)
{
    int clkTicks = sysconf(_SC_CLK_TCK);
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

    for (int i = 0; i < count; i++)
    {
        deleteBlock(blockArray, i + startIndex);
        if (i + startIndex >= blockArray->size)
        {
            return;
        }
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
    int opt, firstFilenameIndex, lastFilenameIndex, count, idx;
    BlockArray *blockArray = NULL;
    reportsFile = fopen("raport2.txt", "w+");
    printReportHeader();
    while ((opt = getopt(argc, argv, "cdegirstw")) != -1)
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
                deleteBlock(blockArray, atoi(argv[optind++]));
            }
            else
            {
                printf("Missing argument, usage: -r INDEX\n");
                return -1;
            }
            break;
        case 's':
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
                printf("Block content: %s\n", getBlock(blockArray, atoi(argv[optind++])));
            }
            break;
        case 't':
            startTimer();
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
        default:
            printf("Unrecognized option: -%c\n", optopt);
            break;
        }
    }
    fclose(reportsFile);
    deleteBlockArray(blockArray);
    return 0;
}