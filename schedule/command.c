#include<command.h>
#include <start32.h>
#include <system.h>
#include <io.h>
#include <printk.h>
#include <tty.h>
#include <console.h>
#include <sched.h>
#include <mem.h>

static int do_help(int argc, char *argv[]);
static int do_task_stat(int argc, char *argv[]);
static int do_cls(int argc, char *argv[]);
static cmd_t g_cmd_table[] ={
    {"help", do_help},
    {"stat", do_task_stat},
    {"cls", do_cls},
    {"", NULL }
};
static int do_help(int argc, char *argv[])
{
    int i;

    for(i=0;i<argc;i++) {    
        printk("argv[%d]:%s\n", i, argv[i]);

    }
    
    return 0;
}

static int do_task_stat(int argc, char **argv)
{
    printk("jiffies:%d\n", jiffies);
    return 0;
}

static int do_cls(int argc, char **argv)
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

static int make_argv(char *s, int argvsz, char *argv[])
{
	int argc = 0;

	/* split into argv */
	while (argc < argvsz - 1) {

		/* skip any white space */
		while ((*s == ' ') || (*s == '\t'))
			++s;

		if (*s == '\0')	/* end of s, no more args	*/
			break;

		argv[argc++] = s;	/* begin of argument string	*/

		/* find end of string */
		while (*s && (*s != ' ') && (*s != '\t'))
			++s;

		if (*s == '\0')		/* end of s, no more args	*/
			break;

		*s++ = '\0';		/* terminate current arg	 */
	}
	argv[argc] = NULL;

	return argc;
}
int do_cmd(char *cmd) 
{
    int len;
    int argc = 0;
   char *argv[MAX_ARGV_NUM + 1];	  
            
    /*解析参数和命令*/
    len = strlen(cmd);

    if( len > MAX_COMMAND_LENGTH) {
        printk("command is too long with length:%d\n", len);
        return -1;
    }

    argc = make_argv(cmd, sizeof(argv)/sizeof(argv[0]), argv);
    cmd_t *command = &g_cmd_table[0];
    /*执行命令*/
    while(command->funp) {
        len = strlen(command->cmd) > strlen(argv[0]) ? strlen(argv[0]) : strlen(command->cmd);

        if( !strncmp(command->cmd, argv[0], len)) {
            if( command->funp ) {
                return command->funp( argc, argv);
            }
            else {
                return -1;
            }
        }
        command++;
    }
    printk("command: %s not found\n", argv[0]);
    return -1;

}


