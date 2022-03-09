#include "library.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    printf("%d arguments passed.\n", argc);
    int currentArg = 1;
    int opt, firstFilenameIndex, lastFilenameIndex;
    while ((opt = getopt(argc, argv, "cwr")) != -1)
    {
        switch (opt)
        {
        case 'c':
            if (optind < argc)
            {
                createTable(atoi(argv[optind++]));
            }
            else
            {
                printf("Missing argument, usage: create SIZE\n");
                return 1;
            }
            break;
        case 'w':
            firstFilenameIndex = optind;
            lastFilenameIndex = 0;
            while (optind < argc)
            {
                if (strcmp(argv[optind], "-c") == 0 || strcmp(argv[optind], "-w") == 0 || strcmp(argv[optind], "-r") == 0)
                {
                    break;
                }
                lastFilenameIndex = ++optind;
            }
            printf("opcja w dla indeksow %d %d \n", firstFilenameIndex, lastFilenameIndex);
            if (lastFilenameIndex > firstFilenameIndex)
            {
                char **filenames = (char **)calloc(lastFilenameIndex - firstFilenameIndex, sizeof(char *));
                memcpy(filenames, argv + firstFilenameIndex, (lastFilenameIndex - firstFilenameIndex) * sizeof(char *));
                countFiles(lastFilenameIndex - firstFilenameIndex, filenames);
            }
            break;
        case 'r':
            break;
        default:
            printf("Unrecognized option: %c\n", optopt);
            break;
        }
    }
    // while (currentArg < argc)
    // {
    //     printf("%d %s\n", currentArg, argv[currentArg]);
    //     if (strcmp(argv[currentArg], "create") == 0)
    //     {
    //         if (currentArg < argc - 1)
    //         {
    //             createTable(atoi(argv[++currentArg]));
    //             currentArg++;
    //             continue;
    //         }
    //         else
    //         {
    //             printf("Missing argument, usage: create SIZE\n");
    //             return 1;
    //         }
    //     }
    //     if (strcmp(argv[currentArg], "count") == 0)
    //     {
    //         if (currentArg < argc - 1)
    //         {
    //             wcFile(argv[++currentArg]);
    //             currentArg++;
    //             continue;
    //         }
    //         else
    //         {
    //             printf("Missing argument, usage: count FILE\n");
    //             return 1;
    //         }
    //     }
    //     else
    //     {
    //         printf("Unrecognized parameter.\n");
    //         return 1;
    //     }
    //     // if (strcmp(argv[currentArg], "remove") == 0)
    //     // {
    //     //     if (currentArg < argc - 1)
    //     //     {
    //     //         createTable(atoi(argv[++currentArg]));
    //     //         currentArg++;
    //     //     }
    //     //     else
    //     //     {
    //     //         printf("Missing argument, usage: remove INDEX\n");
    //     //         return 1;
    //     //     }
    //     // }
    // }
    return 0;
}