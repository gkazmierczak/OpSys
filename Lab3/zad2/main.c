#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/times.h>
#include <float.h>

static clock_t startTime, endTime;
static struct tms st_cpu, en_cpu;

void startTimer(void)
{
    startTime = times(&st_cpu);
}

void endTimer(void)
{
    endTime = times(&en_cpu);
}

void saveTimer(int childrenCount, char *prec, FILE *reportsFile)
{
    int clkTicks = sysconf(_SC_CLK_TCK);
    double realTime = (double)(endTime - startTime) / clkTicks;
    double systemTime = (double)(en_cpu.tms_cstime - st_cpu.tms_cstime) / clkTicks;
    double userTime = (double)(en_cpu.tms_cutime - st_cpu.tms_cutime) / clkTicks;
    printf("Real time:  %f | System time:  %f | User time:  %f | Process count:  %d | Precision:  %s \n", realTime, systemTime, userTime, childrenCount, prec);
    fprintf(reportsFile, "%13f | %15f | %13f | %19d |  %s \n", realTime, systemTime, userTime, childrenCount, prec);
}
void calculateRects(unsigned long long startRectIndex, unsigned long long rectCount, double rectWidth, int childNumber)
{
    double total = 0;
    double rectValue;
    double x;

    for (unsigned long long j = startRectIndex; j <= startRectIndex + rectCount; j++)
    {
        x = (double)(j * rectWidth) + (double)(rectWidth / 2);
        rectValue = (double)(4.0 / ((x * x) + 1));
        total += rectValue;
    }
    char *filename = (char *)calloc(8, sizeof(char));
    char num[4];
    filename[0] = 'w';
    sprintf(num, "%d", childNumber);
    strcat(filename, num);
    strcat(filename, ".txt");
    FILE *file = fopen(filename, "w+");
    fprintf(file, "%.*g\n", DBL_DIG + 10, rectWidth * total);
    fflush(file);
    fclose(file);
    free(filename);
    return;
}

void sumResultsFromFiles(int childProcessCount)
{
    char *filename = (char *)calloc(8, sizeof(char));
    size_t len = 0;
    char num[4];
    double total = 0;
    char *line = NULL;
    double partialResult = 0;

    for (int i = 0; i < childProcessCount; i++)
    {
        filename[0] = 'w';
        filename[1] = '\0';
        sprintf(num, "%d", i + 1);
        strcat(filename, num);
        strcat(filename, ".txt");
        FILE *file = fopen(filename, "r");
        getline(&line, &len, file);
        partialResult = strtod(line, NULL);
        total += partialResult;
        fclose(file);
    }
    free(filename);
    free(line);
    printf("RESULT: %.*g\n", DBL_DIG + 10, total);
    fflush(stdout);
}

int main(int argc, char **argv)
{
    double rectWidth = strtod(argv[1], NULL);
    int childProcessCount = atoi(argv[2]);
    unsigned long long rectsPerChild = ((1 / rectWidth) / childProcessCount);
    unsigned long long rectIndex = 0;
    int pid = 0;
    FILE *reportsFile = fopen("results.txt", "a+");
    startTimer();
    for (int i = 0; i < childProcessCount; i++)
    {
        pid = fork();
        if (pid == 0)
        {
            calculateRects(rectIndex, rectsPerChild, rectWidth, i + 1);
            return 0;
        }
        else
        {
            rectIndex += rectsPerChild + 1;
        }
    }
    for (int i = 0; i < childProcessCount; i++)
    {
        wait(0);
    }
    endTimer();
    sumResultsFromFiles(childProcessCount);
    saveTimer(childProcessCount, argv[1], reportsFile);
    fclose(reportsFile);
    return 0;
}