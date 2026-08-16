#ifndef SHELL_H
#define SHELL_H


#define MAX_INPUT_LENGTH 128



typedef enum {
    COMMAND_UNKNOWN = -1,
    HELP,
    ECHO,
    REBOOT,
    SHUTDOWN,
    CLEAR
} CommandType;



#define MAX_ARGS_COUNT 10


void ShellProcess(void);

char getc(void);
void gets(char *buffer);

void ParseCommand(
    CommandType type,
    char *args[MAX_ARGS_COUNT]
);


#endif
