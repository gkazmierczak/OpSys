#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <sys/times.h>
#include <sys/types.h>

#define BUFFER_SIZE 1024

static clock_t startTime, endTime;
static struct tms st_cpu, en_cpu;
FILE *reportsFile;

long min(long a, long b)
{
    if (a > b)
        return b;
    return a;
}

void lookupCharLib(char lookup, char *filename)
{
    FILE *src = fopen(filename, "r");
    if (src == NULL)
    {
        perror("Error opening file: ");
        exit(-1);
    }
    char lineBuffer[BUFFER_SIZE];
    long charCount = 0;
    long lineCount = 0;
    int containsChar = 0;
    size_t bufferLen = fread(lineBuffer, sizeof(char), BUFFER_SIZE, src);
    while (bufferLen > 0)
    {
        for (int i = 0; i < bufferLen; i++)
        {
            if (lineBuffer[i] == '\n')
            {
                if (containsChar)
                {
                    lineCount++;
                }
                containsChar = 0;
            }
            if (lineBuffer[i] == lookup)
            {
                charCount++;
                containsChar = 1;
            }
        }
        bufferLen = fread(lineBuffer, sizeof(char), BUFFER_SIZE, src);
    }
    if (containsChar)
    {
        lineCount++;
    }
    printf("LIB: Found: %ld   Lines: %ld\n", charCount, lineCount);
    fclose(src);
}

void lookupCharSys(char lookup, char *filename)
{
    int src = open(filename, O_RDONLY);
    if (src < 0)
    {
        perror("Error opening file: ");
        exit(-1);
    }
    char lineBuffer[BUFFER_SIZE];
    long charCount = 0;
    long lineCount = 0;
    int containsChar = 0;
    size_t bufferLen = read(src, lineBuffer, BUFFER_SIZE);
    while (bufferLen > 0)
    {
        for (int i = 0; i < bufferLen; i++)
        {
            if (lineBuffer[i] == '\n')
            {
                if (containsChar)
                {
                    lineCount++;
                }
                containsChar = 0;
            }
            if (lineBuffer[i] == lookup)
            {
                charCount++;
                containsChar = 1;
            }
        }
        bufferLen = read(src, lineBuffer, BUFFER_SIZE);
    }
    if (containsChar)
    {
        lineCount++;
    }
    printf("SYS: Found: %ld   Lines: %ld\n", charCount, lineCount);
    close(src);
}

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

void saveTimer(char *method, char *filename)
{
    int clkTicks = sysconf(_SC_CLK_TCK);
    double realTime = (double)(endTime - startTime) / clkTicks;
    double systemTime = (double)(en_cpu.tms_cstime - st_cpu.tms_cstime) / clkTicks;
    double userTime = (double)(en_cpu.tms_cutime - st_cpu.tms_cutime) / clkTicks;
    fprintf(reportsFile, "%13f | %15f | %13f | Method: %s   Input file: %s \n", realTime, systemTime, userTime, method, filename);
}

void handleTimerEnd(char *method, char *testName)
{
    endTimer();
    if (startTime != 0)
    {
        saveTimer(method, testName);
    }
    return;
}

int main(int argc, char **argv)
{
    char *filename = malloc(128 * sizeof(char));
    char lookup;

    if (argc >= 3)
    {
        lookup = argv[1][0];
        filename = argv[2];
    }
    else if (argc == 2)
    {
        lookup = argv[1][0];
        printf("Enter file name: ");
        scanf("%s", filename);
    }
    else
    {
        printf("Enter character to lookup: ");
        scanf("%c", &lookup);
        printf("Enter file name: ");
        scanf("%s", filename);
    }

    reportsFile = fopen("results.txt", "a+");
    printReportHeader();
    startTimer();
    lookupCharSys(lookup, filename);
    handleTimerEnd("sys", filename);
    startTimer();
    lookupCharLib(lookup, filename);
    handleTimerEnd("lib", filename);
    fclose(reportsFile);
    return 0;
}
