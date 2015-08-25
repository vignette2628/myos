#ifndef __COMMAND_H__
#define __COMMAND_H__

#define MAX_COMMAND_LENGTH  32
#define MAX_ARGV_NUM  3

typedef struct cmd_s {
    char *cmd;
    int (*funp)(int argc, char *argv[]);
} cmd_t;

int do_cmd(char *cmd);
#endif
