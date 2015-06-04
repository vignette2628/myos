#include <system.h>
#include <io.h>
#include <tty.h>
#include <console.h>
#include <printk.h>
//屏幕大小
static unsigned long g_screen_top = 0;//第几行，默认是第0行
static unsigned long g_screen_bottom = DISP_MAX_Y;//第几行，默认是第25行
//字符坐标
static unsigned long g_x = 1; //x坐标 
static unsigned long g_y = 1; //y坐标
//光标
static unsigned long g_position = DISP_BASE_ADDR; // 记录当前光标在显示内存中的位置
//屏幕位置
static unsigned long g_screen_origin = DISP_BASE_ADDR; //记录屏幕在显示内存中的起始位置
static unsigned long g_screen_end = DISP_BASE_ADDR + BYTES_FOR_ONE_LINE*DISP_MAX_Y; //记录屏幕在显示内存中的结束位置
static unsigned char g_color = DISP_FG_WHITE + DISP_BG_BLACK; 
/*
DISP_BASE_ADDR
g_screen_origin  +--------------------+
                            |g_screen_top                  |
                            |x,y 相对于屏幕     |
                            |                                        |
                            |                                        |
                            |                                        |
                            +--------------------+  <---- g_screen_end
                            |g_screen_bottom             |
                            |                                         |
                            +--------------------+DISP_END_ADDR

*/
//设置屏幕的起始位置
static void set_screen_origin(unsigned char flag)
{
    if( flag )
        cli();
    outb_p(12, REG_PORT);
    outb_p(0xff&((g_screen_origin - DISP_BASE_ADDR) >> 9), VALUE_PORT);
    outb_p(13, REG_PORT);
    outb_p(0xff &((g_screen_origin - DISP_BASE_ADDR)>>1), VALUE_PORT);

    if( flag )
        sti();
}

void clear_screen(unsigned color)
{
    unsigned char *p;
    unsigned short i;
    unsigned int length = ( g_screen_bottom -g_screen_top) * BYTES_FOR_ONE_LINE; 
    p = (unsigned char *)g_screen_origin;
    for(i = 0; i <  length;i++) {
        *p++ = ' ';
        *p++ = color;
    }

    return;
}

 inline void gotoxy(unsigned int new_x,unsigned int new_y)
{
        //确保坐标在合理范围内
	if (new_x > DISP_MAX_X || new_y >= DISP_MAX_Y)
		return;
         /*刷新x，y坐标和显示内存中位置偏移。
            y表示行号，x表示列号(x<<1)的目的:是一个字符+颜色占据两个字节。
         */
	g_x=new_x;
	g_y=new_y;
	g_position=g_screen_origin + g_y*BYTES_FOR_ONE_LINE + (g_x<<1);
}

//在(g_x,g_y)坐标处设定光标
inline void set_cursor(unsigned char flag)
{
        if( flag)
	    cli();
        
	outb_p(14, REG_PORT);
	outb_p(0xff&((g_position-DISP_BASE_ADDR)>>9), VALUE_PORT);
	outb_p(15, REG_PORT);
	outb_p(0xff&((g_position-DISP_BASE_ADDR)>>1), VALUE_PORT);
    
        if( flag )
	    sti();
}


void console_init(void)
{
    /*设置屏幕的起始位置*/
    (void)set_screen_origin(0);
    /*清屏*/
    (void)clear_screen(DISP_FG_WHITE + DISP_BG_BLACK);
    (void)gotoxy(0, 0);
    /*设置光标*/
    (void)set_cursor(0);
}
/*屏幕向上翻一行，等同于屏幕向下移动一行*/
static void screen_up(void)
{
    int i;
    unsigned char *src;
    unsigned char *dest;
    unsigned int len;
    /*是否需要整个屏幕向上翻一行*/
    if ( g_screen_top == 0 && g_screen_bottom == DISP_MAX_Y ) {      
        //刷新屏幕在显存中位置
        g_screen_origin += BYTES_FOR_ONE_LINE;
        g_screen_end += BYTES_FOR_ONE_LINE;
        /*刷新光标位置为换行*/
        g_position += BYTES_FOR_ONE_LINE; 
        if( g_screen_end > DISP_END_ADDR )  {/*如果屏幕在显存中位置超出了显存本身的范围*/
            /*将屏幕内容刷新到DISP_BASE_ADDR处*/
            src = (unsigned char *)g_screen_origin;
            dest = (unsigned char *)DISP_BASE_ADDR;
            len = (g_screen_end - g_screen_origin);
            for(i = 0; i < len; i++) {
                dest[i] = src[i];
            }
            /*刷新光标位置和屏幕起始和结束地址*/
            g_screen_end = DISP_BASE_ADDR + len;
            g_position = DISP_BASE_ADDR + (g_position - g_screen_origin);
            g_screen_origin = DISP_BASE_ADDR;
            
        }
        else {
            
        }
        set_screen_origin(1);
    }
    else { /*不支持*/

    }
}
/*屏幕向下翻一行*/
static void screen_down(void)
{
}
static void add_a_line(void)
{
    /*如果不是最底下的一行*/
    if( g_y + 1 < DISP_MAX_Y ) {
        g_y++;
        g_position += BYTES_FOR_ONE_LINE; 
        return;
    }
    /*否则就要刷新整个屏幕的数据*/
    screen_up();
}
//用空格字符填充被删除的字符
static void delete_char(void)
{
    if( g_x > 0 ) {
        g_position -= 2;
        g_x--;
        *(unsigned short *)g_position = ((DISP_FG_WHITE + DISP_BG_BLACK) << 8 )|(0x20);//0x20是空格字符
    }
    return;
}



void console_write(void *tty, char *buf )
{
    unsigned int number = 0;
    unsigned char ch;
    unsigned char *up;
    
    if( !tty || !buf )
        return;
    
    /* 统计当前write_q队列中的字符数*/
    number = ((((tty_struct_t *)tty)->write_q.head-((tty_struct_t *)tty)->write_q.tail)&(TTY_BUF_SIZE-1));
    /* 读取并处理所有write_q中的字符*/
    while( number-- ) {
        ch =   get_char_from_queue( &((tty_struct_t *)tty)->write_q);   
        if( ch > 31 && ch < 127 ) { /*非控制字符*/
            if( g_x >=  DISP_MAX_X) {
                g_x  = g_x - DISP_MAX_X;//换行并进一行
                g_position = g_position - BYTES_FOR_ONE_LINE;
                add_a_line();
            }
            //刷新g_position以便后面刷新光标位置
            up = (unsigned char *)g_position;
            up[0] = ch; //字符
            up[1] = g_color; //颜色
            g_position += 2;
            g_x++;
        }
        else if(ch == 10 || ch== 11 || ch== 12 || ch == 13) { /*换行'\n' 并重新调整坐标*/
           add_a_line();
           gotoxy(0, g_y);
        }
        else if ( ch == 177 ||ch==127 ) { /*删除符号*/
            delete_char();
        }
        else {
            /*不支持*/
        }
    }
    /*刷新光标位置*/
    (void)set_cursor(1);
}
