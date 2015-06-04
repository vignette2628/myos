#include <system.h>
#include <keyboard.h>
#include <io.h>
#include <pci_regs.h>
#include <sched.h>
#include <printk.h>
#include <mem.h>
#include <tty.h>
#include <console.h>

/*正常情况下key与ascii的映射表*/
static unsigned char g_key2ascii_normal_map_table[] = {
    0, 27, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 127, 9,
    'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', 0, 10, 0, 'a', 's',
    'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0, '\\', 'z', 'x', 'c',
    'v', 'b', 'n', 'm', ',', '.', '/', 0, '*', 0, 32, 16, 1, 0, '-', 0, 0, 0,
    '+', 0, 0, 0, 0, 0, 0, 0, '<', 10, 1, 0
};

/*按下shift键后key与ascii的映射表*/
static unsigned char g_key2ascii_shift_map_table[] = {
    0, 27, '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', 127, 9,
    'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', 10, 0, 'A', 'S',
    'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0, '|', 'Z', 'X', 'C',
    'V', 'B', 'N', 'M', '<', '>', '?', 0, '\'', '*', 0, 32, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, '-', 0, 0, 0, '+', 0, 0, 0, 0, 0, 0, 0, '>', 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0
};

/*按下alt之后的key与ascii的映射表  */
static unsigned char g_key2ascii_alt_map_table[] = {
    0, 0, 0, '@', 0, '$', 0, 0, '{', '[', ']', '}', '\\', 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, '~', 10, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, '|', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

static unsigned char g_number_table[] = {
    '7', '8', '9', ' ', '4', '5', '6', ' ', '1', '2', '3', '0'
};

static unsigned int g_current_mode = 0;
static unsigned char g_caps_led_status = 0;

int self_func(unsigned char scan_code)
{
    unsigned char *key_map = &g_key2ascii_normal_map_table[0];
    unsigned char ch;
    unsigned short length = sizeof(g_key2ascii_normal_map_table);
    tty_struct_t *tty;
    int i;
    
    /*只处理非键盘键的0~9数字，a~z字母，shift键和Capslock键，
       回车键和回退键,其他默认返回
       只支持按下键后又放开，不支持持续按键
   */
    if( g_current_mode & SHIFT_MODE ) {
        if( !(g_current_mode & CAPS_MODE ) ) {
            key_map = &g_key2ascii_shift_map_table[0];
            length = sizeof(g_key2ascii_shift_map_table);
 
        }
    }
    else if ( g_current_mode & CAPS_MODE ) {
        key_map = &g_key2ascii_shift_map_table[0];
        length = sizeof(g_key2ascii_shift_map_table);
    }

    if( scan_code >= length )
        return 0;
    
    ch = key_map[scan_code & 0xff];
    /*只支持字母、数字和空格*/
    if( ( ch <= 'Z' && ch >= 'A' ) || ( ch <= 'z' && ch >= 'a' ) ||( ch <= '9' && ch >= '0' ) ||ch == 127  || ch == 10 || ch == ' ') {
        
        /*入console队列并打印*/        
        tty_write(get_tty(CONSOLE_ID), (char *)&ch, 1 );
        if( g_ttx_task ) {
            tty = get_tty(g_ttx_task->tty_channel_id);
            //printk("tty:%x\n",(unsigned int)tty);
        if( !tty )
            return 0;
        /*处理后入read_q供任务读取并处理*/
        
        if (((tty->read_q.tail-tty->read_q.head-1)&(TTY_BUF_SIZE-1))) {
            put_char_into_queue(tty, ch);
            //printk("put %d into read_q with [%d>>%d]\n", ch, tty->read_q.head, tty->read_q.tail);
            wmb();
            return 0;
            }
        }
    }
    return 0;
}



/*键盘中断处理程序*/
void keyboard_interrupt(void)
{
    /*1、从端口 0x60读取扫描码
        2、写控制寄存器端口0x61，复位键盘触发器
        3、复位8259，以便接收下一个键盘中断信号
    */
    unsigned char scan_code;
    scan_code = inb_p(0x60);
    static unsigned char towice = 0;
    //printk("code:%x, mode:%x\n", scan_code, g_current_mode);
    switch (scan_code) {
        case press_alt:/*alt键被按下*/
            g_current_mode |= ALT_MODE;
            break;
            
        case release_alt:
            g_current_mode &= ~ALT_MODE;
            break;
            
        case press_left_shift: /*shift键被按下*/
            g_current_mode |= SHIFT_MODE;
            break;
            
        case press_right_shift:
            g_current_mode |= SHIFT_MODE;
            break;

        case release_left_shift: 
            g_current_mode &= ~SHIFT_MODE;
            break;
            
        case release_right_shift:
            g_current_mode &= ~SHIFT_MODE;
            break;
            
        case press_ctrl: /*ctrl键被按下*/
            g_current_mode |= CTRL_MODE;
            break;

        case release_ctrl: 
            g_current_mode &= ~CTRL_MODE;
            break;
            
        case press_Caps: /*Caps Lock键被按下*/
            towice = (towice+1)%2;
            if( towice)
                break;
            
            if ( g_current_mode & CAPS_MODE ) {
                g_current_mode &= ~CAPS_MODE;
            }
            else {
                g_current_mode |= CAPS_MODE;
            }
            break;

        case release_Caps:

            break;

        case press_Num:
                /*不支持数字键盘*/
            break;

        case release_Num:
            /*不支持数字键盘*/
            break;
            
        default:
            self_func(scan_code );
            break;
    }

    outb_p((inb_p(0x61)) | 0x80, 0x61);
    outb_p((inb_p(0x61)) & 0x7f, 0x61);

    outb_p(0x20, 0x20);

    return;
    
}
