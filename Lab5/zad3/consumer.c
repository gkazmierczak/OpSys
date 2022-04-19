#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/file.h>

void saveMessage(FILE *file, char *data, int num)
{

    flock(fileno(file), LOCK_EX);
    rewind(file);
    char c;
    int lineNum = 0;
    int appendIdx = -2;
    while (fread(&c, sizeof(char), 1, file) > 0)
    {
        if ((c == '\n' || c == 0x0) && lineNum == num)
        {
            appendIdx = ftell(file) - 1;
        }
        if (c == '\n')
            lineNum++;
    }
    fseek(file, 0, SEEK_END);
    size_t filesize = ftell(file);
    if (appendIdx != -2)
    {
        fseek(file, appendIdx, SEEK_SET);
        char *contentRemaining = malloc(sizeof(char) * (filesize - appendIdx));
        fread(contentRemaining, sizeof(char), filesize - appendIdx, file);
        fseek(file, appendIdx, SEEK_SET);
        fwrite(data, sizeof(char), strlen(data), file);
        fwrite(contentRemaining, sizeof(char), filesize - appendIdx, file);
        fflush(file);
        free(contentRemaining);
    }
    else
    {
        while (lineNum < num)
        {
            fwrite("\n", sizeof(char), 1, file);
            lineNum++;
        }
        fwrite(data, sizeof(char), strlen(data), file);
        fflush(file);
    }
    fseek(file, 0, SEEK_SET);
    flock(fileno(file), LOCK_UN);
}

void consumerLoop(char *fifoPath, char *filepath, int bufferSize)
{
    char *data = (char *)calloc(bufferSize + 5, sizeof(char));
    FILE *file = fopen(filepath, "w+");
    if (!file)
    {
        fprintf(stderr, "Error opening file.\n");
        fflush(stderr);
        exit(-1);
    }
    FILE *fifo = fopen(fifoPath, "r");
    if (!fifo)
    {
        fprintf(stderr, "Error opening pipe.\n");
        fflush(stderr);
        fclose(file);
        exit(-1);
    }
    setvbuf(fifo, NULL, _IONBF, 0);
    int num;
    while (flock(fileno(fifo), LOCK_EX) == 0 && fread(data, sizeof(char), bufferSize, fifo) > 0)
    {
        flock(fileno(fifo), LOCK_UN);
        char *row = strtok(data, "_");
        num = atoi(row);
        char *content = strtok(NULL, "\n");
        if (content == NULL)
        {
            continue;
        }
        saveMessage(file, content, num);
    }
    free(data);
    fclose(fifo);
    fclose(file);
}

int main(int argc, char **argv)
{
    if (argc == 4)
    {
        if (atoi(argv[3]) < 1)
        {
            fprintf(stderr, "Number of bytes per read must be greater than 0.\n");
            fflush(stderr);
            exit(-1);
        }
        consumerLoop(argv[1], argv[2], atoi(argv[3]));
    }
    else
    {
        fprintf(stderr, "Incorrect number of arguments. Usage: consumer FIFO_PATH OUTPUT_PATH N\n");
        fflush(stderr);
        return -1;
    }
    return 0;
}