#ifndef __SCHED_H__
#define __SCHED_H__
#define HZ 100

#define delay(ticks) __asm__ __volatile__ ("cli"::); \
                                 current->sleep_time =  (ticks); \
                                 __asm__ __volatile__ ("sti"::);    \
                                 __asm__ __volatile__ ("int $0x80"::);
                                 
#define LATCH ((1193180 + HZ/2) / HZ)

#define MAX_TASK_NUMBER 31
#define TASK_STK_SIZE 8192

#define TASK_RUNNING		0
#define TASK_INTERRUPTIBLE	1
#define TASK_UNINTERRUPTIBLE	2
#define _get_base(addr) ({\
unsigned long __base; \
__asm__("movb %3,%%dh\n\t" \
	"movb %2,%%dl\n\t" \
	"shll $16,%%edx\n\t" \
	"movw %1,%%dx" \
	:"=d" (__base) \
	:"m" (*((addr)+2)), \
	 "m" (*((addr)+4)), \
	 "m" (*((addr)+7))); \
__base;})
#define get_base(ldt) _get_base( ((char *)&(ldt)) )

typedef struct stack_s {
    long * esp;
    short ss;
} stack_t;

#define MAX_CBUF_NUM 1024
typedef struct my_queue_s {
	unsigned long head;
	unsigned long tail;
	unsigned int buf_list[MAX_CBUF_NUM];
}my_queue_t;

typedef struct cbuf_s{
    unsigned int length;
    void *message;
}cbuf_t;

typedef struct task_struct_s{
    unsigned char task_id;
    unsigned int *task_func_p;
    unsigned int *task_stk;
    int budget;
    int counter;
    unsigned int state;
    unsigned int    current_esp;
    unsigned int sleep_time;
    unsigned int stat_counter;
    unsigned char tty_channel_id;
    my_queue_t cbuf_queue;
    my_queue_t mbuf_queue;
}task_struct_t;

typedef struct lock_s {
    unsigned char locked; /*0:Ã»Ëø, 1:Ëø*/
    task_struct_t *wait_list;
}lock_t;

extern task_struct_t *current;
extern task_struct_t *g_ttx_task;
extern long volatile jiffies;
unsigned int schedule(void);
void sleep_on( task_struct_t **p);
void wake_up(task_struct_t **p);

#endif
