#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void testFile(char *inputpath, char *outputpath, int lineNum, int maxLineLen)
{
    FILE *fp1 = fopen(inputpath, "r");
    if (!fp1)
    {
        fprintf(stderr, "Error opening file.\n");
        fflush(stderr);
        exit(-1);
    }
    FILE *fp2 = fopen(outputpath, "r");
    if (!fp2)
    {
        fprintf(stderr, "Error opening file.\n");
        fflush(stderr);
        fclose(fp1);
        exit(-1);
    }
    int inputSize;
    fseek(fp1, 0, SEEK_END);
    inputSize = ftell(fp1);
    rewind(fp1);
    char *inputContent = (char *)calloc((inputSize + 2), sizeof(char));
    fread(inputContent, sizeof(char), inputSize, fp1);
    char *outputLine = (char *)calloc((maxLineLen + 2), sizeof(char));
    int idx = 0;
    while (idx < lineNum)
    {
        fgets(outputLine, maxLineLen + 2, fp2);
        idx++;
    }
    memset(outputLine, 0, maxLineLen);
    fgets(outputLine, inputSize + 1, fp2);
    for (int i = 0; i < inputSize; i++)
    {
        if (inputContent[i] != outputLine[i])
        {
            printf("IN:  %s\n", inputContent);
            printf("OUT: %s\n", outputLine);
            printf("Output file content is INCORRECT.\n");
            fflush(stdout);
            exit(0);
        }
    }
    printf("Output file content is CORRECT.\n");
    fflush(stdout);
}

int main(int argc, char **argv)
{
    if (argc == 5)
    {
        testFile(argv[1], argv[2], atoi(argv[3]), atoi(argv[4]));
    }
    else
    {
        fprintf(stderr, "Incorrect number of arguments. Usage: tester INPUT OUTPUT LINE_NUM MAX_LINE_LEN\n");
        fflush(stderr);
        return -1;
    }
    return 0;
}