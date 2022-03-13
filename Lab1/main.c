#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char **argv)
{
    int opt, firstFilenameIndex, lastFilenameIndex, count;
    BlockArray *blockArray;
    while ((opt = getopt(argc, argv, "cwrsi")) != -1)
    {
        switch (opt)
        {
        case 'c':
            if (optind < argc)
            {
                blockArray = initBlockArray(atoi(argv[optind++]));
            }
            else
            {
                printf("Missing argument, usage: -c SIZE\n");
                return -1;
            }
            break;
        case 'w':
            firstFilenameIndex = optind;
            lastFilenameIndex = 0;
            while (optind < argc)
            {
                if (argv[optind][0] == '-')
                {
                    break;
                }
                lastFilenameIndex = ++optind;
            }
            if (lastFilenameIndex > firstFilenameIndex)
            {
                count = lastFilenameIndex - firstFilenameIndex;
                countFiles(count, firstFilenameIndex, argv);
            }
            else
            {
                printf("Missing argument, usage: -w FILENAME\n");
                return -1;
            }
            break;
        case 'r':
            if (optind < argc)
            {
                deleteBlock(blockArray, atoi(argv[optind++]));
            }
            else
            {
                printf("Missing argument, usage: -r INDEX\n");
                return -1;
            }
            break;
        case 's':
            printf("Tempfile content stored at index: %d\n", storeTempfile(blockArray));
            break;
        default:
            printf("Unrecognized option: -%c\n", optopt);
            break;
        }
    }
    return 0;
}

char *generateRandomString(int size)
{
    if (size < 1)
        return NULL;
    char *base = "0123456789QWERTYUIOPASDFGHJKLZXCVBNMqwertyuiopasdfghjklzxcvbnm _.";
    char *result = (char *)calloc(strlen(base), sizeof(char));
    for (int i = 0; i < size - 1; i++)
    {
        result[i] = base[rand() % strlen(base)];
    }
    result[size - 1] = '\0';
    return result;
}

void insertNBlocks(BlockArray *blockArray, int count, int startIndex, int blockSize)
{
    if (count > blockArray->size)
    {
        printf("Incorrect argument: Cannot insert %d blocks into %d-sized BlockArray.\n", count, blockArray->size);
        return;
    }
    char *data = generateRandomString(blockSize);
    for (int i = 0; i < count; i++)
    {
        insertIntoArrayAt(blockArray, i + startIndex, data);
    }
}

void deleteNBlocks(BlockArray *blockArray, int count, int startIndex)
{
    if (count > blockArray->size)
    {
        printf("Incorrect argument: Cannot delete %d blocks from %d-sized BlockArray.\n", count, blockArray->size);
        return;
    }

    for (int i = 0; i < count; i++)
    {
        deleteBlock(blockArray, i + startIndex);
    }
}