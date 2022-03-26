#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <sys/times.h>

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

void saveTimer(char *prec, FILE *reportsFile)
{
    int clkTicks = sysconf(_SC_CLK_TCK);
    double realTime = (double)(endTime - startTime) / clkTicks;
    double systemTime = (double)(en_cpu.tms_cstime - st_cpu.tms_cstime) / clkTicks;
    double userTime = (double)(en_cpu.tms_cutime - st_cpu.tms_cutime) / clkTicks;
    printf("%13f | %15f | %13f | Precision: %s \n", realTime, systemTime, userTime, prec);
    fprintf(reportsFile, "%13f | %15f | %13f | Precision: %s \n", realTime, systemTime, userTime, prec);
}
void printReportHeader(FILE *reportsFile)
{
    fprintf(reportsFile, "\nReal time [s] | System time [s] | User time [s] | Precision  \n");
}
void calculateRects(int startRectIndex, int rectCount, double rectWidth, int childNumber)
{
    double total = 0;
    double rectValue;
    double x;
    for (int j = startRectIndex; j <= startRectIndex + rectCount; j++)
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
    fprintf(file, "%f\n", rectWidth * total);
    fflush(file);
    fclose(file);
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
        printf("%s\n", filename);
        fflush(stdout);
        getline(&line, &len, file);

        printf("%s\n", line);
        fflush(stdout);
        partialResult = strtod(line, NULL);
        total += partialResult;
        fclose(file);
    }
    printf("TOTAL: %f\n", total);
    fflush(stdout);
}

int main(int argc, char **argv)
{
    double rectWidth = strtod(argv[1], NULL);
    int childProcessCount = atoi(argv[2]);
    long rectsPerChild = ((1 / rectWidth) / childProcessCount);
    int rectIndex = 0;
    int pid;
    FILE *reportsFile = fopen("results.txt", "a+");
    printReportHeader(reportsFile);
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
        /
    }
    for (int i = 0; i < childProcessCount; i++)
    {
        wait(0);
    }
    endTimer();
    sumResultsFromFiles(childProcessCount);
    saveTimer(argv[1], reportsFile);
    fclose(reportsFile);
    return 0;
}