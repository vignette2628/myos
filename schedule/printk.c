#include <printk.h>
#include <stdarg.h>
#include <mem.h>
#include <console.h>
#include <tty.h>

static char *g_print_buf = (char *)MEM_PRINTK_BUF;

/*console初始化前的调试打印调用函数。在x ,y坐标处打印带颜色的一个字符*/
void printk_char(unsigned char x, unsigned char y, unsigned char c, unsigned char color)
{
    unsigned short offset;
    unsigned char *p;
    
    offset = (unsigned short)y * DISP_MAX_X *2 + (unsigned short)x * 2;
    p = (unsigned char *) (DISP_BASE_ADDR + offset);
    *p++ = c;
    *p++ = color;
    
    return;
}

/*console初始化前的调试打印调用函数。在x ,y坐标起始处打印带颜色的一串字符*/
void printk_string(unsigned char x, unsigned char y, unsigned char *s, unsigned char color)
{
    unsigned short offset;
    unsigned char *p;
    
    if( !s)
        return;
    
    offset = (unsigned short)y*DISP_MAX_X*2 + (unsigned short)x*2;
    p = (unsigned char *)(DISP_BASE_ADDR+offset);
    while(*s) {
        *p++=*s++;
        *p++= color;
    }
    return;
}


void clear_row(unsigned char y,unsigned char color)
{
    unsigned char *p;
    unsigned char i;

    p = (unsigned char *)(DISP_BASE_ADDR + (unsigned short)(y*DISP_MAX_X * 2) );
    for( i =0; i < DISP_MAX_X; i++) {
        *p++ = ' ';
        *p++ = color;
    }
    
    return;
}
int snprintf(char *buf, unsigned int length, const char *fmt, ...)
{
    int i, j;
    va_list args;
    for(i = 0; i < length; i++) {
        buf[i] = '\0';
    }
    va_start(args, fmt);
    j = vsprintf(buf, fmt, args);
    va_end(args);
    if (g_print_buf[j-1] == '\n') {
        j--;
    }

    return j;
}

void printk_init(void)
{
    int i;
    for(i=0;i<MAX_PRINT_BUF;i++) {
        memset(g_print_buf, 0x0, MAX_PRINT_BUF);
    }
}


int printk(const char *fmt, ...)
{
    va_list args;
    int i;
    char ch;
    unsigned char *up = (unsigned char *)&g_print_buf[0];
    
    va_start(args, fmt);
    i = vsprintf(g_print_buf, fmt, args);
    va_end(args);
    
    tty_write(get_tty(CONSOLE_ID), up, ((i > MAX_PRINT_BUF) ? MAX_PRINT_BUF: i) );
}
