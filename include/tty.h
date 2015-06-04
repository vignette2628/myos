#ifndef __TTY_H__
#define __TTY_H__

#define TTY_BUF_SIZE 1024
enum {
    CONSOLE_ID= 0,
    MAX_CHANNEL_ID
};

typedef struct tty_queue_s {
	unsigned long data;
	unsigned long head;
	unsigned long tail;
	char buf[TTY_BUF_SIZE];
}tty_queue_t;

typedef struct tty_struct_s {
    tty_queue_t read_q;
    tty_queue_t write_q;
    void (*write)(void *tty, char *buf);
    unsigned int task_id;
}tty_struct_t;

void tty_init(void);
tty_struct_t *get_tty(unsigned int channel_id);
unsigned char get_char_from_queue(tty_queue_t *q);
#endif
