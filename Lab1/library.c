#include "library.h"

#include <stdio.h>
#include <stdlib.h>

void **createTable(long size)
{
    void **array;
    array = (void **)calloc(size, sizeof(void *));
    printf("Allocated memory. \n");
    return array;
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
    // FILE *temp = fopen("wc.tmp", "a");
    // if (temp == NULL)
    // {
    //     printf("Error opening temp file.");
    //     return -1;
    // }
    // fprintf(temp, "%ld %ld %ld %s", lines, words, chars, filename);
    // fclose(temp);

    return 0;
}
int countFiles(int filecount, char **filenames)
{
    FILE *temp = fopen("wc.tmp", "w+");
    for (int i = 0; i < filecount; i++)
    {
        printf("%s \n", filenames[i]);
        wcFile(filenames[i], temp);
    }
    fclose(temp);
    return 0;
}