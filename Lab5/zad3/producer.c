#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define min(a, b) a > b ? b : a

void productionLoop(char *fifoPath, char *row, char *filepath, int bufferSize)
{
    char *data = (char *)calloc(bufferSize, sizeof(char));
    char *content = (char *)calloc(bufferSize + strlen(row) + 1, sizeof(char));
    int writeLen = bufferSize + strlen(row) + 1;
    FILE *file = fopen(filepath, "r");
    if (!file)
    {
        fprintf(stderr, "Error opening file.\n");
        fflush(stderr);
        exit(-1);
    }
    FILE *fifo = fopen(fifoPath, "w");
    if (!fifo)
    {
        fprintf(stderr, "Error opening pipe.\n");
        fflush(stderr);
        fclose(file);
        exit(-1);
    }
    // Disable fifo buffer, prevents intersecting messages when N>PIPE_BUF
    setvbuf(fifo, NULL, _IONBF, 0);
    size_t rd;
    while ((rd = fread(data, sizeof(char), bufferSize, file)) > 0)
    {
        // If less than bufferSize were read, fill remaining data with spaces.
        if (rd < bufferSize)
        {
            for (size_t i = rd; i < bufferSize; i++)
            {
                data[i] = 0;
            }
        }
        sprintf(content, "%s_%s", row, data);
        usleep((0.01 + (rand() / RAND_MAX)) * 1000000);
        fwrite(content, sizeof(char), writeLen, fifo);
    }
    free(data);
    free(content);
    fclose(fifo);
    fclose(file);
}

int main(int argc, char **argv)
{
    if (argc == 5)
    {
        productionLoop(argv[1], argv[2], argv[3], atoi(argv[4]));
    }
    else
    {
        fprintf(stderr, "Incorrect number of arguments. Usage: producer FIFO_PATH ROW_NUMBER OUTPUT_PATH N\n");
        fflush(stderr);
        return -1;
    }
    return 0;
}