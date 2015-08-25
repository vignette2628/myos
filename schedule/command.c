#include<command.h>
#include <start32.h>
#include <system.h>
#include <io.h>
#include <printk.h>
#include <tty.h>
#include <console.h>
#include <sched.h>

static int do_help(char *cmd);
static int do_task_stat(char *cmd);
static int do_cls(char *cmd);
static cmd_t g_cmd_table[] ={
    {"help", do_help},
    {"stat", do_task_stat},
    {"cls", do_cls},
    {"", NULL }
};
static int do_help(char *cmd)
{
    cmd_t *command = &g_cmd_table[0];
    printk("commands:\n");
    while(command->funp) {
        printk("%s\n", command->cmd);
        command++;
    }
    return 0;
}

static int do_task_stat(char *cmd)
{
    printk("jiffies:%d\n", jiffies);
    return 0;
}

static int do_cls(char *cmd)
{
    (void)clear_screen(DISP_FG_WHITE + DISP_BG_BLACK);
    gotoxy(0,0);
    set_cursor(1);
    
    return 0;
}

static int strncmp(char *src, char *dest, unsigned int len )
{
    unsigned int i;
    for(i=0;i<len;i++) {
        if( src[i] != dest[i] )
            return 1;
    }

    return 0;
}

static int strlen(char *string)
{
    int i;
    
    if( !string)
        return 0;
    i = 0;
    while(string[i] != '\0'){
        i++;
    }

    return i;
}

int do_cmd(char *cmd) 
{
    int len;
    
    cmd_t *command = &g_cmd_table[0];
    
    while(command->funp) {
        len = strlen(command->cmd) > strlen(cmd) ? strlen(cmd) : strlen(command->cmd);
        if( !strncmp(command->cmd, cmd, len)) {
            
            return command->funp(cmd);
        }
        command++;
    }
    printk("command not found\n");
    return -1;
}


