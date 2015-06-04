#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#define COLOR_VGA_MODE

/*不检查显卡模式，强制写死*/
#ifdef COLOR_VGA_MODE
#define  DISP_BASE_ADDR 0xB8000       /* 0xB8000=VGA基地址 ,彩色模式*/
#define  DISP_END_ADDR  0xba000      
#define  DISP_MAX_X         80       /* 每行80个字符*/
#define  DISP_MAX_Y         25       /* 1屏最大有25行*/
#define  BYTES_FOR_ONE_LINE (DISP_MAX_X <<1)

#define  REG_PORT       0x3d4      /*显示寄存器索引控制端口地址*/
#define VALUE_PORT    0x3d5  /*数据寄存器端口地址*/
#endif


void console_init(void);
void console_write(void *tty, char *buf );
void clear_screen(unsigned color);
inline void set_cursor(unsigned char flag);
inline void gotoxy(unsigned int new_x,unsigned int new_y);
#endif
