#include <system.h>
#include <io.h>
#include <tty.h>
#include <console.h>
#include <printk.h>
//��Ļ��С
static unsigned long g_screen_top = 0;//�ڼ��У�Ĭ���ǵ�0��
static unsigned long g_screen_bottom = DISP_MAX_Y;//�ڼ��У�Ĭ���ǵ�25��
//�ַ�����
static unsigned long g_x = 1; //x���� 
static unsigned long g_y = 1; //y����
//���
static unsigned long g_position = DISP_BASE_ADDR; // ��¼��ǰ�������ʾ�ڴ��е�λ��
//��Ļλ��
static unsigned long g_screen_origin = DISP_BASE_ADDR; //��¼��Ļ����ʾ�ڴ��е���ʼλ��
static unsigned long g_screen_end = DISP_BASE_ADDR + BYTES_FOR_ONE_LINE*DISP_MAX_Y; //��¼��Ļ����ʾ�ڴ��еĽ���λ��
static unsigned char g_color = DISP_FG_WHITE + DISP_BG_BLACK; 
/*
DISP_BASE_ADDR
g_screen_origin  +--------------------+
                            |g_screen_top                  |
                            |x,y �������Ļ     |
                            |                                        |
                            |                                        |
                            |                                        |
                            +--------------------+  <---- g_screen_end
                            |g_screen_bottom             |
                            |                                         |
                            +--------------------+DISP_END_ADDR

*/
//������Ļ����ʼλ��
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
        //ȷ�������ں���Χ��
	if (new_x > DISP_MAX_X || new_y >= DISP_MAX_Y)
		return;
         /*ˢ��x��y�������ʾ�ڴ���λ��ƫ�ơ�
            y��ʾ�кţ�x��ʾ�к�(x<<1)��Ŀ��:��һ���ַ�+��ɫռ�������ֽڡ�
         */
	g_x=new_x;
	g_y=new_y;
	g_position=g_screen_origin + g_y*BYTES_FOR_ONE_LINE + (g_x<<1);
}

//��(g_x,g_y)���괦�趨���
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
    /*������Ļ����ʼλ��*/
    (void)set_screen_origin(0);
    /*����*/
    (void)clear_screen(DISP_FG_WHITE + DISP_BG_BLACK);
    (void)gotoxy(0, 0);
    /*���ù��*/
    (void)set_cursor(0);
}
/*��Ļ���Ϸ�һ�У���ͬ����Ļ�����ƶ�һ��*/
static void screen_up(void)
{
    int i;
    unsigned char *src;
    unsigned char *dest;
    unsigned int len;
    /*�Ƿ���Ҫ������Ļ���Ϸ�һ��*/
    if ( g_screen_top == 0 && g_screen_bottom == DISP_MAX_Y ) {      
        //ˢ����Ļ���Դ���λ��
        g_screen_origin += BYTES_FOR_ONE_LINE;
        g_screen_end += BYTES_FOR_ONE_LINE;
        /*ˢ�¹��λ��Ϊ����*/
        g_position += BYTES_FOR_ONE_LINE; 
        if( g_screen_end > DISP_END_ADDR )  {/*�����Ļ���Դ���λ�ó������Դ汾��ķ�Χ*/
            /*����Ļ����ˢ�µ�DISP_BASE_ADDR��*/
            src = (unsigned char *)g_screen_origin;
            dest = (unsigned char *)DISP_BASE_ADDR;
            len = (g_screen_end - g_screen_origin);
            for(i = 0; i < len; i++) {
                dest[i] = src[i];
            }
            /*ˢ�¹��λ�ú���Ļ��ʼ�ͽ�����ַ*/
            g_screen_end = DISP_BASE_ADDR + len;
            g_position = DISP_BASE_ADDR + (g_position - g_screen_origin);
            g_screen_origin = DISP_BASE_ADDR;
            
        }
        else {
            
        }
        set_screen_origin(1);
    }
    else { /*��֧��*/

    }
}
/*��Ļ���·�һ��*/
static void screen_down(void)
{
}
static void add_a_line(void)
{
    /*�����������µ�һ��*/
    if( g_y + 1 < DISP_MAX_Y ) {
        g_y++;
        g_position += BYTES_FOR_ONE_LINE; 
        return;
    }
    /*�����Ҫˢ��������Ļ������*/
    screen_up();
}
//�ÿո��ַ���䱻ɾ�����ַ�
static void delete_char(void)
{
    if( g_x > 0 ) {
        g_position -= 2;
        g_x--;
        *(unsigned short *)g_position = ((DISP_FG_WHITE + DISP_BG_BLACK) << 8 )|(0x20);//0x20�ǿո��ַ�
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
    
    /* ͳ�Ƶ�ǰwrite_q�����е��ַ���*/
    number = ((((tty_struct_t *)tty)->write_q.head-((tty_struct_t *)tty)->write_q.tail)&(TTY_BUF_SIZE-1));
    /* ��ȡ����������write_q�е��ַ�*/
    while( number-- ) {
        ch =   get_char_from_queue( &((tty_struct_t *)tty)->write_q);   
        if( ch > 31 && ch < 127 ) { /*�ǿ����ַ�*/
            if( g_x >=  DISP_MAX_X) {
                g_x  = g_x - DISP_MAX_X;//���в���һ��
                g_position = g_position - BYTES_FOR_ONE_LINE;
                add_a_line();
            }
            //ˢ��g_position�Ա����ˢ�¹��λ��
            up = (unsigned char *)g_position;
            up[0] = ch; //�ַ�
            up[1] = g_color; //��ɫ
            g_position += 2;
            g_x++;
        }
        else if(ch == 10 || ch== 11 || ch== 12 || ch == 13) { /*����'\n' �����µ�������*/
           add_a_line();
           gotoxy(0, g_y);
        }
        else if ( ch == 177 ||ch==127 ) { /*ɾ������*/
            delete_char();
        }
        else {
            /*��֧��*/
        }
    }
    /*ˢ�¹��λ��*/
    (void)set_cursor(1);
}
