#include "library.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEMP_FILE "results.tmp"

BlockArray *initBlockArray(int size)
{
    if (size <= 0)
        return NULL;
    BlockArray *blockArray = calloc(1, sizeof(BlockArray));
    blockArray->size = size;
    blockArray->array = (void **)calloc(size, sizeof(void *));
    return blockArray;
}

int wcFile(char *filename, FILE *tempfile)
{
    unsigned long chars = 0, words = 0, lines = 0;
    FILE *file = fopen(filename, "r");
    if (file == NULL)
    {
        printf("Could not open file %s .\n", filename);
        return -1;
    }
    char c;
    while ((c = fgetc(file)) != EOF)
    {
        chars++;
        if (c == '\n' || c == ' ' || c == '\0' || c == '\t')
            words++;
        if (c == '\n' || c == '\0')
            lines++;
    }
    fclose(file);
    if (tempfile != NULL)
    {
        fprintf(tempfile, "%ld %ld %ld %s\n", lines, words, chars, filename);
    }
    return 0;
}

int countFiles(int filecount, int startIndex, char **args)
{
    FILE *temp = fopen(TEMP_FILE, "w+");
    for (int i = 0; i < filecount; i++)
    {
        printf("%s \n", args[startIndex + i]);
        wcFile(args[startIndex + i], temp);
    }
    fclose(temp);
    return 0;
}

char *getTempfileContent(void)
{
    FILE *temp = fopen(TEMP_FILE, "r");
    if (temp == NULL)
    {
        printf("No tempfile found.");
        return NULL;
    }
    fseek(temp, 0L, SEEK_END);
    long filesize = ftell(temp);
    fseek(temp, 0L, SEEK_SET);
    char *fileContent = (char *)calloc(filesize + 1, sizeof(char));
    fread(fileContent, sizeof(char), filesize, temp);
    fileContent[filesize] = '\0';
    fclose(temp);
    return fileContent;
}

int storeTempfile(BlockArray *blockArray)
{
    // Check if BlockArray was initialised
    if (blockArray == NULL || blockArray->array == NULL)
        return -1;

    // Lookup free block index
    int blockIndex = 0;
    while (blockArray->array[blockIndex] != NULL && blockIndex < blockArray->size)
    {
        blockIndex++;
    }

    // No free block found
    if (blockIndex > blockArray->size)
        return -2;

    // Get content of tempfile, store it in array and return index
    char *fileContent = getTempfileContent();
    blockArray->array[blockIndex] = fileContent;
    return blockIndex;
}

const char *getBlock(BlockArray *blockArray, int index)
{
    if (blockArray == NULL || blockArray->array == NULL || index < 0 || index >= blockArray->size)
        return NULL;
    return blockArray->array[index];
}

int deleteBlock(BlockArray *blockArray, int index)
{
    // Check edge cases
    if (blockArray == NULL || blockArray->array == NULL)
        return -1;
    if (index < 0 || index >= blockArray->size)
        return -2;
    if (blockArray->array[index] == NULL)
        return 0;

    // Free memory block
    free(blockArray->array[index]);
    blockArray->array[index] = NULL;
    return 0;
}

int insertIntoArrayAt(BlockArray *blockArray, int index, void *data)
{
    //  Check edge cases
    if (blockArray == NULL || blockArray->array == NULL)
        return -1;
    if (index < 0 || index >= blockArray->size)
        return -2;
    if (blockArray->array[index] != NULL)
        return -3;
    blockArray->array[index] = data;
    return 0;
}
