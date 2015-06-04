#include <stdarg.h>
#include <mem.h>
#include <tty.h>
#include <console.h>
#include <system.h>
static  tty_struct_t g_tty_table[MAX_CHANNEL_ID];

void tty_init(void)
{
    int i;
    
    for(i=0;i<MAX_CHANNEL_ID;i++) {
        memset((unsigned char *)&g_tty_table[i], 0x0, sizeof(tty_struct_t));
    }
    g_tty_table[CONSOLE_ID].write = console_write;

}

unsigned char get_char_from_queue(tty_queue_t *q)
{
    unsigned char ch = q->buf[q->tail];
    q->tail = ( q->tail + 1 ) & (TTY_BUF_SIZE-1);

    return ch;
}

void put_char_into_queue(tty_queue_t *q, char ch)
{
    q->buf[q->head] = ch;
    q->head = ( q->head + 1 )&(TTY_BUF_SIZE - 1);

    return;
}
void tty_write(tty_struct_t *tty, char *buf, unsigned int len)
{
    char *up = buf;
    char ch;
    
    if( !tty || !buf  || len == 0 || len > 1024)
        return;

    if( tty->write == NULL ) {
        return;
    }
    
    //入write_q队列
    while(len > 0 && (((tty->write_q.tail-tty->write_q.head-1)&(TTY_BUF_SIZE-1)))) { //保证队列没满
        ch = up[0];
        up++;
        len--;
        tty->write_q.buf[tty->write_q.head] = ch;
        tty->write_q.head = (tty->write_q.head+1) & (TTY_BUF_SIZE-1);
    }

    return tty->write(tty, buf );
    
}

tty_struct_t *get_tty(unsigned int channel_id)
{
    if( channel_id >= MAX_CHANNEL_ID)
        return NULL;

    return &g_tty_table[channel_id];
}
