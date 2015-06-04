#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#define COLOR_VGA_MODE

/*������Կ�ģʽ��ǿ��д��*/
#ifdef COLOR_VGA_MODE
#define  DISP_BASE_ADDR 0xB8000       /* 0xB8000=VGA����ַ ,��ɫģʽ*/
#define  DISP_END_ADDR  0xba000      
#define  DISP_MAX_X         80       /* ÿ��80���ַ�*/
#define  DISP_MAX_Y         25       /* 1�������25��*/
#define  BYTES_FOR_ONE_LINE (DISP_MAX_X <<1)

#define  REG_PORT       0x3d4      /*��ʾ�Ĵ����������ƶ˿ڵ�ַ*/
#define VALUE_PORT    0x3d5  /*���ݼĴ����˿ڵ�ַ*/
#endif


void console_init(void);
void console_write(void *tty, char *buf );
void clear_screen(unsigned color);
inline void set_cursor(unsigned char flag);
inline void gotoxy(unsigned int new_x,unsigned int new_y);
#endif
