#pragma once
#include <stdio.h>
extern void **createTable(long size);
extern int wcFile(char *filename, FILE *tempfile);
extern int countFiles(int filecount, char **filenames);