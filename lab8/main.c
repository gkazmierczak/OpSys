#include <stdio.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

int **inputMatrix;
int **outputMatrix;
int imageWidth;
int imageHeight;
int maxVal;
struct threadArg
{
    int from;
    int to;
};

void loadImage(int threadCount, int mode, char *filename)
{
    FILE *file = fopen(filename, "r");
    char *buff = (char *)malloc(512 * sizeof(char));
    char *saveptr;
    fgets(buff, 32, file);
    fgets(buff, 512, file);
    fgets(buff, 32, file);
    imageWidth = atoi(strtok_r(buff, " ", &saveptr));
    imageHeight = atoi(saveptr);
    buff = (char *)realloc(buff, imageWidth * 5 * sizeof(char));
    fgets(buff, imageWidth * 5, file);
    maxVal = atoi(buff);
    inputMatrix = (int **)calloc(imageHeight, sizeof(int *));
    outputMatrix = (int **)calloc(imageHeight, sizeof(int *));
    for (int i = 0; i < imageHeight; i++)
    {
        inputMatrix[i] = (int *)calloc(imageWidth, sizeof(int));
        outputMatrix[i] = (int *)calloc(imageWidth, sizeof(int));
    }
    char *tmp;
    char *rest;
    int x = 0;
    int y = 0;
    while (fgets(buff, imageWidth * 5, file))
    {
        rest = buff;
        tmp = strtok_r(rest, " \t\r\n", &rest);
        inputMatrix[y][x] = atoi(tmp);
        x++;
        if (x >= imageWidth)
        {
            x = 0;
            y++;
        }
        while ((tmp = strtok_r(rest, " \t\r\n", &rest)) != NULL)
        {

            inputMatrix[y][x] = atoi(tmp);
            x++;

            if (x >= imageWidth)
            {
                x = 0;
                y++;
            }
        }
    }
    fclose(file);
    free(buff);
}

void saveImage(char *filename)
{
    FILE *file = fopen(filename, "w");
    if (file == NULL)
    {
        perror("Cannot open output file");
        exit(-1);
    }

    fprintf(file, "P2\n");
    fprintf(file, "%d %d\n", imageWidth, imageHeight);
    fprintf(file, "255\n");

    for (int i = 0; i < imageHeight; i++)
    {
        for (int j = 0; j < imageWidth; j++)
        {
            fprintf(file, "%d ", outputMatrix[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}
void *numbers(void *arg)
{

    struct timeval start, end;
    gettimeofday(&start, NULL);
    struct threadArg *args = ((struct threadArg *)arg);
    for (int i = 0; i < imageHeight; i++)
    {
        for (int j = 0; j < imageWidth; j++)
        {
            if (inputMatrix[i][j] >= args->from && inputMatrix[i][j] < args->to)
            {
                outputMatrix[i][j] = maxVal - inputMatrix[i][j];
            }
        }
    }

    gettimeofday(&end, NULL);
    long unsigned int *time = malloc(sizeof(long unsigned int));
    *time = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
    pthread_exit(time);
}
void *blocks(void *arg)
{

    struct timeval start, end;
    gettimeofday(&start, NULL);
    struct threadArg *args = ((struct threadArg *)arg);
    for (int i = 0; i < imageHeight; i++)
    {
        for (int j = args->from; j <= args->to; j++)
        {
            outputMatrix[i][j] = maxVal - inputMatrix[i][j];
        }
    }

    gettimeofday(&end, NULL);
    long unsigned int *time = malloc(sizeof(long unsigned int));
    *time = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
    pthread_exit(time);
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        fprintf(stderr, "Incorrect number of arguments. Usage: ./main THREADS MODE INPUT_PATH OUTPUT_PATH\n");
        exit(-1);
    }
    int threadCount = atoi(argv[1]);
    char *inputPath = argv[3];
    char *outputPath = argv[4];
    loadImage(threadCount, 0, inputPath);
    struct timeval start, end;
    gettimeofday(&start, NULL);

    pthread_t *threads = (pthread_t *)calloc(threadCount, sizeof(pthread_t));
    struct threadArg *threadArgs = (struct threadArg *)calloc(threadCount, sizeof(struct threadArg));
    if (!strcasecmp(argv[2], "NUMBERS"))
    {
        for (int i = 0; i < threadCount; i++)
        {
            threadArgs[i].from = i * (maxVal / threadCount);
            if (i != threadCount - 1)
            {
                threadArgs[i].to = (i + 1) * (maxVal / threadCount);
            }
            else
            {
                threadArgs[i].to = maxVal + 1;
            }
            pthread_create(&threads[i], NULL, numbers, &threadArgs[i]);
        }
    }
    else if (!strcasecmp(argv[2], "BLOCK"))
    {
        for (int i = 0; i < threadCount; i++)
        {
            threadArgs[i].from = i * ceil(imageWidth / threadCount);
            if (i != threadCount - 1)
            {
                threadArgs[i].to = (i + 1) * ceil(imageWidth / threadCount) - 1;
            }
            else
            {
                threadArgs[i].to = imageWidth - 1;
            }

            pthread_create(&threads[i], NULL, blocks, &threadArgs[i]);
        }
    }
    else
    {
        fprintf(stderr, "Wrong mode. Available modes: NUMBERS, BLOCK\n");
        exit(-1);
    }
    FILE *timesFile = fopen("times.txt", "a+");
    if (timesFile == NULL)
    {
        perror("Could not open times file ");
        puts("Times will be printed to stdout instead.");
    }

    for (int i = 0; i < threadCount; i++)
    {
        long unsigned int *time;
        pthread_join(threads[i], (void **)&time);
        if (timesFile != NULL)
        {
            fprintf(timesFile, "Thread #%d | Mode: %s | Input file path: %s | Thread execution time: %lu [us]\n", i, argv[2], inputPath, *time);
        }
        else
        {
            printf("Thread #%d | Mode: %s | Input file path: %s | Thread execution time: %lu [us]\n", i, argv[2], inputPath, *time);
        }
        free(time);
    }
    gettimeofday(&end, NULL);
    long unsigned int totalTime = (end.tv_sec - start.tv_sec) * 1000000 + end.tv_usec - start.tv_usec;
    if (timesFile != NULL)
    {
        fprintf(timesFile, "Mode: %s | Thread count: %d | Input file path: %s | Total execution time: %lu [us]\n\n\n", argv[2], threadCount, inputPath, totalTime);
    }
    else
    {
        printf("Mode: %s | Thread count: %d | Input file path: %s | Total execution time: %lu [us]\n\n\n", argv[2], threadCount, inputPath, totalTime);
    }

    saveImage(outputPath);
    free(threadArgs);
    free(threads);
    for (int i = 0; i < imageHeight; i++)
    {
        free(inputMatrix[i]);
        free(outputMatrix[i]);
    }
    free(inputMatrix);
    free(outputMatrix);
}