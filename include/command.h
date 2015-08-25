#ifndef __COMMAND_H__
#define __COMMAND_H__
typedef struct cmd_s {
    char *cmd;
    int (*funp)(char *cmd);
} cmd_t;

int do_cmd(char *cmd);
#endif
