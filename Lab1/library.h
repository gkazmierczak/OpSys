#pragma once
#include <stdio.h>

typedef struct BlockArray
{
    int size;
    void **array;
} BlockArray;

/*
    Initialises BlockArray of specified size
*/
extern BlockArray *initBlockArray(int size);

/*
    Performs wc-like count on a specified file, storing the results in tempfile
    Returns:
        0 - Success
        -1 - Failed to open specified file
*/
extern int wcFile(char *filename, FILE *tempfile);

/*
    Handles calling wcFile for files specified in **filenames
*/
extern int countFiles(int filecount, int startIndex, char **args);

/*
    Returns contents of specified BlockArray's block with given index or NULL if BlockArray was not initialised or index is out of bounds
*/
const char *getBlock(BlockArray *blockArray, int index);

/*
    Frees up memory taken by specified block in a BlockArray.
    Returns:
        0 - Success
        -1 - Specified BlockArray was not initialised
        -2 - Index out of bounds
*/
extern int deleteBlock(BlockArray *blockArray, int index);

/*
    Reads the content of tempfile and returns it as char*
*/
char *getTempfileContent(void);

/*
    Attempts to store contents of tempfile into specified BlockArray at first free block
    Returns:
        int >=0 -  index at which the contents were stored
        -1 - Specified BlockArray was not initialised
        -2 - Specified BlockArray is already full
*/
extern int storeTempfile(BlockArray *blockArray);

/*
    Attempts to store passed data block in specified BlockArray at given index, does not overwrite data already present
    Returns:
        0 - Success
        -1 - Specified BlockArray was not initialised
        -2 - Index out of bounds
        -3 - Some data already occupies specified index in BlockArray
*/
extern int insertIntoArrayAt(BlockArray *blockArray, int index, void *data);
