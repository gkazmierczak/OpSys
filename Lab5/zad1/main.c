#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/wait.h>

#define MAX_LINES 100
#define MAX_COMMANDS 100
#define MAX_ARGS 32
#define MAX_LINE_LENGTH 2048
#define IN 0
#define OUT 1

typedef struct
{
    char *name;       // Line name (eg. s1)
    char **commands;  // Commands list (in execution order)
    char ***args;     // Command arguments (in command execution order)
    int commandCount; // Count of commands to allow for easy iteration
} parsed_line;

int getLineNumber(char *line)
{
    /*
        Return number at the end of string up to 4 digits (eg. sample123 returns 123)
    */
    char *numberString = (char *)calloc(5, sizeof(char));
    int i = 0;
    for (size_t j = 0; j < strlen(line); j++)
    {
        if (isdigit(line[j]))
            numberString[i++] = line[j];
        if (i >= 4)
            break;
    }
    numberString[i] = '\0';
    int res = atoi(numberString);
    free(numberString);
    return res;
}

void parseArguments(parsed_line *parsedLine)
{
    /*
        Parse component into commands and their arguments, discarding empty arguments.
    */
    char *commandWithArgs;
    char *comm;
    char *arg;
    int argCount;
    char ***args = (char ***)calloc(MAX_COMMANDS, sizeof(char **));
    for (int i = 0; i < parsedLine->commandCount; i++)
    {
        argCount = 0;
        args[i] = (char **)calloc(MAX_ARGS, sizeof(char *));
        commandWithArgs = parsedLine->commands[i];
        comm = strtok(commandWithArgs, " ");
        if (comm[strlen(comm) - 1] == '\n')
            comm[strlen(comm) - 1] = '\0';
        parsedLine->commands[i] = comm;
        args[i][argCount++] = comm;
        while ((arg = strtok(NULL, " ")) != NULL)
        {
            if (!strcmp(arg, "\n"))
            {
                arg = NULL;
            }
            else if (arg[strlen(arg) - 1] == '\n')
            {
                arg[strlen(arg) - 1] = '\0';
            }

            args[i][argCount++] = arg;
        }
        args[i][argCount] = NULL;
    }
    parsedLine->args = args;
    return;
}

void printParsedLine(parsed_line *line)
{
    /*
        Prints component in more readable format.
    */
    puts(line->name);
    printf("Command count: %d\n", line->commandCount);
    int j;
    for (int i = 0; i < line->commandCount; i++)
    {
        j = 0;
        printf("Command: %s\n", line->commands[i]);
        while (line->args[i][j] != NULL)
        {
            printf("- Arg %d : %s \n", j, line->args[i][j]);
            j++;
        }
    }
    printf("\n");
    fflush(stdout);
}

void freeParsedLines(parsed_line **parsedLines, int lineCount)
{
    for (int i = 0; i < lineCount; i++)
    {
        free(parsedLines[i]->name);

        for (int j = 0; j < parsedLines[i]->commandCount; j++)
        {
            free(parsedLines[i]->args[j]);
        }
        free(parsedLines[i]->commands);
        free(parsedLines[i]->args);
        free(parsedLines[i]);
    }
    free(parsedLines);
}

parsed_line *parseLine(char *line)
{
    /*
        Convert char* containing commands to parsed_line conatining parsed data.
    */
    parsed_line *parsedLine = malloc(sizeof(parsed_line));
    char **commands = (char **)calloc(MAX_COMMANDS, sizeof(char *));
    char *comm = strtok(line, "=");
    int commCount = 0;
    while ((comm = strtok(NULL, "|")) != NULL)
    {
        commands[commCount++] = comm;
    }
    parsedLine->name = line;
    parsedLine->commands = commands;
    parsedLine->commandCount = commCount;
    parseArguments(parsedLine);
    return parsedLine;
}

void executeCommands(int *lineIndexes, parsed_line **parsedLines, int componentCount)
{
    /*
        Handles execution of component lines.
        Pipes are swapped between processes, first child does not use pipe input, last child does not use pipe output.
    */
    int prevPipe[2], currPipe[2];
    int currentLineIndex;
    if (pipe(currPipe) != 0 || pipe(prevPipe) != 0)
    {
        fprintf(stderr, "Error opening pipe.\n");
        fflush(stderr);
    }

    for (int i = 0; i < componentCount; i++)
    {
        currentLineIndex = lineIndexes[i] - 1;
        for (int j = 0; j < parsedLines[currentLineIndex]->commandCount; j++)
        {
            pipe(currPipe);
            if (fork() == 0)
            {
                close(prevPipe[1]);
                dup2(prevPipe[0], STDIN_FILENO);
                if (i != componentCount - 1 || j != parsedLines[currentLineIndex]->commandCount - 1)
                {
                    close(currPipe[0]);
                    dup2(currPipe[1], STDOUT_FILENO);
                }
                if (execvp(parsedLines[currentLineIndex]->commands[j], parsedLines[currentLineIndex]->args[j]) == -1)
                {
                    fprintf(stderr, "Error during executing command \"   %s   \".Terminating.\n", parsedLines[currentLineIndex]->commands[j]);
                    fflush(stderr);
                    exit(-1);
                }
                exit(0);
            }
            else
            {
                close(currPipe[1]);
                prevPipe[0] = currPipe[0];
                prevPipe[1] = currPipe[1];
            }
        }
        while (wait(0) != -1)
            ;
    }
    puts("\nFinished line execution.");
    puts("\n------------------------------------ \n");
}

void handleCommandLine(char *line, parsed_line **parsedLines, int lineCount)
{
    int componentIdx = 0;
    static int lineIndexes[MAX_COMMANDS];
    char **components = (char **)calloc(MAX_COMMANDS, sizeof(char *));
    char *component = strtok(line, "|");
    components[componentIdx++] = component;
    while ((component = strtok(NULL, "|")) != NULL)
    {
        components[componentIdx++] = component;
    }
    for (int i = 0; i < componentIdx; i++)
    {
        lineIndexes[i] = getLineNumber(components[i]);
        if (lineIndexes[i] > MAX_LINES)
        {
            fprintf(stderr, "Incorrect input. Maximum command line count is %d, input requires usage of line %d.\n", MAX_LINES, lineIndexes[i]);
            fflush(stderr);
            freeParsedLines(parsedLines, lineCount);
            free(components);
            free(line);
            exit(-1);
        }
        else if (lineIndexes[i] > lineCount)
        {
            fprintf(stderr, "Incorrect input. %d command lines were declared, input requires usage of line %d.\n", lineCount, lineIndexes[i]);
            fflush(stderr);
            free(line);
            free(components);
            freeParsedLines(parsedLines, lineCount);
            exit(-1);
        }
    }
    executeCommands(lineIndexes, parsedLines, componentIdx);
    free(components);
    return;
}

void handleInputFile(char *filepath)
{
    /*
        Handles reading and executing file.
    */
    FILE *fp = fopen(filepath, "r");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening file.\n");
        fflush(stderr);
        exit(-1);
    }
    parsed_line **parsedLines = (parsed_line **)calloc(MAX_LINES, sizeof(parsed_line *));
    char *line = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
    int lineCount = 0;
    while (fgets(line, MAX_LINE_LENGTH * sizeof(char), fp))
    {
        if (strstr(line, "=") != NULL)
        {
            char *lineCopy = (char *)calloc(MAX_LINE_LENGTH, sizeof(char));
            strcpy(lineCopy, line);
            parsed_line *result;
            result = parseLine(lineCopy);
            parsedLines[lineCount++] = result;
        }
        else if (strcmp(line, "\n") != 0)
        {
            if (line[strlen(line) - 1] == '\n')
            {
                line[strlen(line) - 1] = '\0';
            }
            printf("Executing line: %s\nOutput:\n", line);
            fflush(stdout);
            handleCommandLine(line, parsedLines, lineCount);
        }
    }
    fclose(fp);
    free(line);
    freeParsedLines(parsedLines, lineCount);
}

int main(int argc, char **argv)
{
    if (argc == 2)
    {
        handleInputFile(argv[1]);
    }
    else
    {
        fprintf(stderr, "Missing argument. Usage: zad1 FILE\n");
        fflush(stderr);
        return -1;
    }
    return 0;
}