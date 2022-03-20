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

void stripWhitelinesLib(char *srcFilename, char *destFilename)
{
    FILE *src = fopen(srcFilename, "r");
    FILE *dest = fopen(destFilename, "w+");
    if (src == NULL || dest == NULL)
    {
        perror("Error opening files: ");
        exit(-1);
    }
    char lineBuffer[BUFFER_SIZE];
    char tmpBuffer[BUFFER_SIZE];
    long lastInterestPointPos = ftell(src);
    long currentBufferStartPos = ftell(src);
    int containsChars = 0;
    size_t bufferLen = fread(lineBuffer, sizeof(char), BUFFER_SIZE, src);
    long lineLen;
    while (bufferLen > 0)
    {
        for (int i = 0; i < bufferLen; i++)
        {

            if (lineBuffer[i] == '\n')
            {
                if (containsChars)
                {
                    if (lastInterestPointPos >= currentBufferStartPos)
                    {
                        lineLen = currentBufferStartPos + i - lastInterestPointPos;
                        fwrite(lineBuffer + i - lineLen, sizeof(char), lineLen, dest);
                        lastInterestPointPos = currentBufferStartPos + i;
                    }
                    else
                    {
                        lineLen = currentBufferStartPos + i - lastInterestPointPos;
                        fseek(src, lastInterestPointPos, SEEK_SET);
                        while (lineLen > 0)
                        {
                            fread(tmpBuffer, sizeof(char), BUFFER_SIZE, src);
                            fwrite(tmpBuffer, sizeof(char), min(lineLen, BUFFER_SIZE), dest);
                            lineLen -= BUFFER_SIZE;
                        }
                        fseek(src, currentBufferStartPos + BUFFER_SIZE, SEEK_SET);
                        lastInterestPointPos = currentBufferStartPos + i;
                    }
                }
                else
                {
                    lastInterestPointPos = currentBufferStartPos + i;
                }
                containsChars = 0;
            }
            if (!isspace(lineBuffer[i]))
            {
                containsChars = 1;
            }
        }
        currentBufferStartPos = ftell(src);
        bufferLen = fread(lineBuffer, sizeof(char), BUFFER_SIZE, src);
    }
    fseek(src, 0, SEEK_END);
    long eofPos = ftell(src);
    if (containsChars)
    {
        fseek(src, lastInterestPointPos, SEEK_SET);
        while (eofPos - ftell(src) > 0)
        {
            bufferLen = fread(lineBuffer, sizeof(char), BUFFER_SIZE, src);
            fwrite(lineBuffer, sizeof(char), bufferLen, dest);
        }
    }
    fclose(src);
    fclose(dest);
}

void stripWhitelinesSys(char *srcFilename, char *destFilename)
{
    int src = open(srcFilename, O_RDONLY);
    int dest = open(destFilename, O_WRONLY | O_CREAT | O_TRUNC);
    if (src < 0 || dest < 0)
    {
        perror("Error opening files: ");
        exit(-1);
    }
    char lineBuffer[BUFFER_SIZE];
    char tmpBuffer[BUFFER_SIZE];
    long lastInterestPointPos = 0;
    long currentBufferStartPos = 0;
    int containsChars = 0;
    size_t bufferLen = read(src, lineBuffer, BUFFER_SIZE);
    long lineLen;
    while (bufferLen > 0)
    {
        for (int i = 0; i < bufferLen; i++)
        {

            if (lineBuffer[i] == '\n')
            {
                if (containsChars)
                {
                    if (lastInterestPointPos >= currentBufferStartPos)
                    {
                        lineLen = currentBufferStartPos + i - lastInterestPointPos;
                        write(dest, lineBuffer + i - lineLen, lineLen);
                        lastInterestPointPos = currentBufferStartPos + i;
                    }
                    else
                    {
                        lineLen = currentBufferStartPos + i - lastInterestPointPos;
                        lseek(src, lastInterestPointPos, SEEK_SET);
                        while (lineLen > 0)
                        {
                            read(src, tmpBuffer, BUFFER_SIZE);
                            write(dest, tmpBuffer, min(lineLen, BUFFER_SIZE));
                            lineLen -= BUFFER_SIZE;
                        }
                        lseek(src, currentBufferStartPos + BUFFER_SIZE, SEEK_SET);
                        lastInterestPointPos = currentBufferStartPos + i;
                    }
                }
                else
                {
                    lastInterestPointPos = currentBufferStartPos + i;
                }
                containsChars = 0;
            }
            if (!isspace(lineBuffer[i]))
            {
                containsChars = 1;
            }
        }
        currentBufferStartPos = lseek(src, 0, SEEK_CUR);
        bufferLen = read(src, lineBuffer, BUFFER_SIZE);
    }
    lseek(src, 0, SEEK_END);
    long eofPos = lseek(src, 0, SEEK_CUR);
    if (containsChars)
    {
        lseek(src, lastInterestPointPos, SEEK_SET);
        while (eofPos - lseek(src, 0, SEEK_CUR) > 0)
        {
            bufferLen = read(src, lineBuffer, BUFFER_SIZE);
            write(dest, lineBuffer, bufferLen);
        }
    }
    close(src);
    close(dest);
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
    char *srcFilename = malloc(50 * sizeof(char)), *destFilename = malloc(50 * sizeof(char));

    if (argc >= 3)
    {
        srcFilename = argv[1];
        destFilename = argv[2];
        printf("%s %s\n", srcFilename, destFilename);
    }
    else if (argc == 2)
    {
        srcFilename = argv[1];
        printf("Enter destination file name: ");
        scanf("%s", destFilename);
        printf("%s %s\n", srcFilename, destFilename);
    }
    else
    {
        printf("Enter source file name: ");
        scanf("%s", srcFilename);
        printf("Enter destination file name: ");
        scanf("%s", destFilename);
        printf("%s %s\n", srcFilename, destFilename);
    }

    reportsFile = fopen("pomiar_zad_1.txt", "a+");
    printReportHeader();
    startTimer();
    stripWhitelinesSys(srcFilename, destFilename);
    handleTimerEnd("sys", srcFilename);
    startTimer();
    stripWhitelinesLib(srcFilename, destFilename);
    handleTimerEnd("lib", srcFilename);
    fclose(reportsFile);
    return 0;
}
