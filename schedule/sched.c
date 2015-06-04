#include <start32.h>
#include <sched.h>
#include <system.h>
#include <io.h>
#include <printk.h>
#include <pcnet32.h>
#include <mem.h>
#include <rx.h>
#include <dhcpc.h>
#include <tty.h>
#include <console.h>
#include <command.h>
#include <hd.h>
extern void timer_interrupt(void);
extern void system_call_interrupt(void);
extern void pcnet32_call_interrupt(void);
extern void keyboard_call_interrupt(void);

/*copy this part of code from linux source code:sched.c*/
#define TIME_REQUESTS 64
static struct timer_list {
	long jiffies;
	void (*fn)();
	struct timer_list * next;
} timer_list[TIME_REQUESTS], * next_timer = NULL;


/*stack for kernel_start()*/    
long user_stack [ TASK_STK_SIZE ] = {0};
stack_t stack_start = { &user_stack [TASK_STK_SIZE] , 0x10 };

/*stack for left tasks: task0, task1, etc...*/
long g_task_stk[MAX_TASK_NUMBER][TASK_STK_SIZE];

long volatile jiffies=0;
long volatile g_max_idle_counter = 0;
long volatile g_idle_counter = 0;
long volatile g_switch_counter = 0;

task_tbl_t g_task_tbl[MAX_TASK_NUMBER+1] = {0};
task_tbl_t *current;
task_tbl_t *g_ttx_task = NULL;

static task_tbl_t  *wait_task = NULL;

void add_timer(long jiffies, void (*fn)(void))
{
	struct timer_list * p;

	if (!fn)
		return;
	cli();
	if (jiffies <= 0)
		(fn)();
	else {
		for (p = timer_list ; p < timer_list + TIME_REQUESTS ; p++)
			if (!p->fn)
				break;
		if (p >= timer_list + TIME_REQUESTS) {
                    printk("No more time requests free\n");
                    return;
                 }
		p->fn = fn;
		p->jiffies = jiffies;
		p->next = next_timer;
		next_timer = p;
		while (p->next && p->next->jiffies < p->jiffies) {
			p->jiffies -= p->next->jiffies;
			fn = p->fn;
			p->fn = p->next->fn;
			p->next->fn = fn;
			jiffies = p->jiffies;
			p->jiffies = p->next->jiffies;
			p->next->jiffies = jiffies;
			p = p->next;
		}
	}
	sti();
}

void save_esp(void)
{
    current->current_esp = get_eax();
    
}

void restore_esp(void)
{
    set_eax(current->current_esp);
    
}

void refresh_sleep_time(void)
{
    int i;
    task_tbl_t *p;
    
    for( i = 0; i <= MAX_TASK_NUMBER;i++) {
        p = &g_task_tbl[i];
        if( p->sleep_time > 0 ) {
            p->sleep_time--;
            if( p->sleep_time == 0) {
                p->counter = p->budget;
            }
        }
    }
    return;
}
/*Ӳ�ж�ʱ�򴥷�����*/
void do_timer(void)
{
    unsigned int task_id;
    int ret;
    static long tmp = 0;
    
    /*ɨ��timer�б�����timer��Ӧ��fn()*/
    if (next_timer) {
        next_timer->jiffies--;
        while (next_timer && next_timer->jiffies <= 0) {
            void (*fn)(void);

            fn = next_timer->fn;
            next_timer->fn = NULL;
            next_timer = next_timer->next;
            (fn)();
        }
    }
    /*ˢ����������� sleep_time�����������ѵ�����ļ���������ΪԤ��ֵ��*/
    refresh_sleep_time();

    /*ˢ�µ�ǰ����ļ�����*/
    if( current->counter > 0 ) {
        current->counter--;      
    }

    /*�ҵ���һ��Ҫִ�е�����ˢ�µ�current��*/
    task_id = schedule();
    current = &g_task_tbl[task_id];
    current->stat_counter++;

    return;
}

/*���ж�ʱ�򴥷�����*/
void do_syscall_timer(void)
{
    unsigned int task_id;
    
    task_id = schedule();
    
    /*�ҵ���һ��Ҫִ�е�����ˢ�µ�current��*/    
    current = &g_task_tbl[task_id];
    current->stat_counter++;
    
    return;
}
static  void task0_timer(void)
{
    add_timer(HZ, task0_timer );
}

void main_task(void)
{
    int cbuf_addr;
    cbuf_t *cbuf;
    my_queue_t *cbuf_queue; 
    my_queue_t *mbuf_queue;

    /*pcnet32 ������ʼ����ͨ����ȡpcie���ÿռ��ȡg_io_address io����ַ���õ�ַ������оƬ�ķ���ʱ��Ҫ�õ�
        ͬʱ��ʼ��init_block�Ա�main_task()����ɹ�����rx��tx��صĲ�����
    */
    (void)pcnet32_drivers_init();
        
    /*Make sure interrupt pin is INTA*/   
    if ( g_card_init_ok == 0 && get_int_pin() == 1) {        
        set_intr_gate((32+get_int_line()), &pcnet32_call_interrupt);//10+32
        outb(inb_p(0x21)&0xfb,0x21);
        outb(inb_p(0xa1)&0xfb,0xa1);   
        cli();
        pcnet32_open(g_io_address);
        sti();
    }

    for(;;){
        /*����cbuf queue*/
        cbuf_queue = &current->cbuf_queue;
        while(( cbuf_queue->head != cbuf_queue->tail )  ) {     
            cbuf_addr = cbuf_queue->buf_list[cbuf_queue->tail];            
            cbuf = (cbuf_t *)cbuf_addr;
            cbuf_queue->tail = (cbuf_queue->tail +1) &(MAX_CBUF_NUM-1);              
        }
        /*����mbuf queue*/
        g_switch_counter = 0;
        delay(HZ);/*1 second*/
    }
}
static void task1_timer(void)
{
    char tmp[256];
    snprintf(tmp, 128, "task1 with counter:%d", g_task_tbl[1].counter);
    
    printk_string(0, 15, tmp, DISP_FG_RED);
    
    add_timer(4000, task1_timer );
}
void rx_task(void)
{
    char tmp[64];  
    int i = 0;
    struct sk_buff *skb;
    for(;;){
        skb = get_rx_skb();
        if( skb ) {
            rx_process(skb);
        }
    }
}

static int get_a_command_line(tty_struct_t *tty, char *buffer)
{
    char ch;
    int len = 0;
    int i;
    
    if( !buffer || !tty )
        return 0;

    while(!( tty->read_q.head == tty->read_q.tail )  ) {
        ch =tty->read_q.buf[tty->read_q.tail];
        printk("ch:%d ", ch);
        tty->read_q.tail = (tty->read_q.tail +1) &(TTY_BUF_SIZE-1);
        wmb();
        if( ch == 10 ) {
            break;
         }
    }
    buffer[len+1] = 0;
    return len;
}
void tty_task(void)
{
    int len = 0;
    char *command_line = (char *)MEM_COMMAND_LINE_BUF;
    int i;
    tty_struct_t *tty;
    current->tty_channel_id = CONSOLE_ID;
    g_ttx_task = current;

    printk("command>");
    for(;;){
        len = 0;
        while(1) {
            tty = get_tty(CONSOLE_ID);
            if(!( tty->read_q.head == tty->read_q.tail )  ) {
                command_line[len] = tty->read_q.buf[tty->read_q.tail];
                tty->read_q.tail = (tty->read_q.tail +1) &(TTY_BUF_SIZE-1);
                wmb();
                if(command_line[len] == 10 ) {
                    command_line[len] = '\0';
                    break;
                }
                len++;                
            }
       
        }           
        do_cmd(command_line);
        printk("command>");

    }
}

unsigned int *task_stk_init(void (*task)(), unsigned int *task_stk )
{
    unsigned int *stk;
    stk = task_stk;

    *stk-- = 0x10; /*ss*/
    *stk-- = (unsigned int)task_stk;   /*esp*/
    *stk-- = 0x206;  /*eflags enable sw interrupt*/    
    *stk-- = 0x08; /*cs*/
    *stk-- = (unsigned int)task; /*eip*/
    
    *stk-- = 0x10; /*ds*/
    *stk-- = 0x10; /*es*/
    *stk-- = 0x10; /*fs*/
    *stk-- = 0x10; /*gs*/
    *stk-- = 0x1111; /*ebp*/
    *stk-- = 0x2222; /*edi*/
    *stk-- = 0x3333; /*esi*/
    *stk-- = 0xdddd; /*edx*/
    *stk-- = 0xcccc; /*ecx*/
    *stk-- = 0xbbbb; /*ebx*/    
    *stk = 0xaaaa;/*eax*/

    return stk;
}


void start_another_task(unsigned char task_id, void *pdata )
{
    
    if( task_id >= MAX_TASK_NUMBER )
        return;
    
    if( !g_task_tbl[task_id].task_func_p || !g_task_tbl[task_id].task_stk ) {
        return;
    }

    /*�����ջ��ʼ��*/
    g_task_tbl[task_id].current_esp = (unsigned int)(task_stk_init((void *)(g_task_tbl[task_id].task_func_p), g_task_tbl[task_id].task_stk ));
    
}

/*���жϷ��ص���,ϵͳ�ĵ�һ������*/
void start_task(void)
{
    int i;    

    /*Ϊ�Լ����������ջ������ָ�����е����ݴ���������g_task_tbl[]ĩβ*/
    g_task_tbl[MAX_TASK_NUMBER].counter = 10;
    g_task_tbl[MAX_TASK_NUMBER].budget= 10;
    g_task_tbl[MAX_TASK_NUMBER].current_esp = get_esp();
    g_task_tbl[MAX_TASK_NUMBER].task_func_p = (unsigned int *)start_task;
    g_task_tbl[MAX_TASK_NUMBER].task_id = MAX_TASK_NUMBER;
    g_task_tbl[MAX_TASK_NUMBER].task_stk = (unsigned int *)(&user_stack [TASK_STK_SIZE]);
    g_task_tbl[MAX_TASK_NUMBER].state = 0;
    memset((unsigned char *)&g_task_tbl[MAX_TASK_NUMBER].cbuf_queue, 0x0, sizeof(my_queue_t));
    memset((unsigned char *)&g_task_tbl[MAX_TASK_NUMBER].mbuf_queue, 0x0, sizeof(my_queue_t));
    
    current = &g_task_tbl[MAX_TASK_NUMBER];
        
    /*Ϊ�������񴴽��������ջ����ز���*/
     for( i = 0 ; i < MAX_TASK_NUMBER; i++ )
    {
        start_another_task(i, NULL);
    }

    /*�ָ��жϣ�֮��cpu���յ�8253ÿ10ms����һ�β������ж��ź��ˡ�*/ 
    sti();
    
    for(;;){
        //idle����ʲô��������
        /*do nothing as idle task*/
    }
}
void sched_init(void)
{
    int i;
    
    for(i = 0; i <= MAX_TASK_NUMBER; i++) {
        memset((unsigned char*)&g_task_tbl[i], 0x0, sizeof(task_tbl_t));
        g_task_tbl[i].task_id = i;
    }
    /*����0*/
    g_task_tbl[0].counter = 10;
    g_task_tbl[0].budget= 10;
    g_task_tbl[0].current_esp = (unsigned int)(&g_task_stk[0][TASK_STK_SIZE-1]);
    g_task_tbl[0].task_func_p = (unsigned int *)main_task;
    g_task_tbl[0].task_stk = (unsigned int *)&g_task_stk[0][TASK_STK_SIZE-1];
    g_task_tbl[0].state = 0;
    memset((unsigned char *)&g_task_tbl[0].cbuf_queue, 0x0, sizeof(my_queue_t));
    memset((unsigned char *)&g_task_tbl[0].mbuf_queue, 0x0, sizeof(my_queue_t));

    /*����1*/
    g_task_tbl[1].counter = 10;
    g_task_tbl[1].budget= 10;
    g_task_tbl[1].current_esp = (unsigned int)(&g_task_stk[1][TASK_STK_SIZE-1]);
    g_task_tbl[1].task_func_p = (unsigned int *)rx_task;
    g_task_tbl[1].task_stk = (unsigned int *)&g_task_stk[1][TASK_STK_SIZE-1];
    g_task_tbl[1].state = 0;
    memset((unsigned char *)&g_task_tbl[1].cbuf_queue, 0x0, sizeof(my_queue_t));
    memset((unsigned char *)&g_task_tbl[1].mbuf_queue, 0x0, sizeof(my_queue_t));

    /*����2*/
    g_task_tbl[2].counter = 10;
    g_task_tbl[2].budget= 10;
    g_task_tbl[2].current_esp = (unsigned int)(&g_task_stk[2][TASK_STK_SIZE-1]);
    g_task_tbl[2].task_func_p = (unsigned int *)tty_task;
    g_task_tbl[2].task_stk = (unsigned int *)&g_task_stk[2][TASK_STK_SIZE-1];
    g_task_tbl[2].state = 0;
    memset((unsigned char *)&g_task_tbl[2].cbuf_queue, 0x0, sizeof(my_queue_t));
    memset((unsigned char *)&g_task_tbl[2].mbuf_queue, 0x0, sizeof(my_queue_t));
    
    clear_nt();

   /*init 8253 chip*/
    outb_p(0x36,0x43);		/* binary, mode 3, LSB/MSB, ch 0 */
    outb_p(LATCH & 0xff , 0x40);	/* LSB */
    outb(LATCH >> 8 , 0x40);	/* MSB */

    /*����ʱ��Ӳ�ж���Ӧ��������һ��8259a�ϵĵ�0���ж���*/
    set_intr_gate(0x20,&timer_interrupt);
    outb(inb_p(0x21)&~0x01,0x21);
    
    /*�������жϵ���Ӧ����*/
    set_intr_gate(0x80, &system_call_interrupt );

    /*���ü����жϵ���Ӧ��������һ��8259a�ϵĵ�1���ж���*/
    set_intr_gate(0x21, &keyboard_call_interrupt);
    outb(inb_p(0x21)&~0x02, 0x21);

}
void sleep_on( task_tbl_t **p)
{
    task_tbl_t *tmp;

    if( !p)
        return;

    if( current == &g_task_tbl[MAX_TASK_NUMBER] ) {
        
        printk("panic: task:%d try to go to sleep.\n", MAX_TASK_NUMBER);
        for(;;);
    }
    tmp = *p;

    *p = current;
    current->state = TASK_UNINTERRUPTIBLE;

    /*���ж����µ�������*/
    int80();
   
    if( tmp)
        tmp->state = TASK_RUNNING;
}

void wake_up(task_tbl_t **p)
{
	if (p && *p) {
		(**p).state=0;
		*p=NULL;
	}
}
/*Ӳ����ʱ���жϻ�0x80���жϴ������������*/
unsigned int schedule(void)
{
    unsigned int i, next;
    int c;
    task_tbl_t *p;
    unsigned int towice = 0;
    int current_id = current->task_id;
    while(1){
        c = -1;
        /*�ҳ���������������(������start_task()����)��˯�߲���couter�����������Ǹ�����*/
        for( i = 0; i < MAX_TASK_NUMBER;i++) {
            p = &g_task_tbl[i];
            
            if( ( p->task_id == i ) && p->task_func_p && ( p->state != TASK_UNINTERRUPTIBLE )) {
                if( p->sleep_time > 0 ) {
                    continue;
                }
                if( p->counter > c ) {
                    c = p->counter;
                    next = i;
                    
                }
            }
        }
        /*������ֵ����0���򷵻����ֵ��Ӧ�����task_id*/
        if(c > 0) {
            break;
        }
        else {
        /*����Ļ�˵�����е�����ļ���������0�ˣ�
         ��Ҫ����Щ����(����������˯�ߵ�����)�ļ�������ˢ�³�Ԥ��ֵ*/
             towice++;
             if( towice > 1 ) {/*ˢ��һ�κ��Ƿ������е�����˯���ˣ���ʱ��ֻ����start_task()����������*/
                if( g_task_tbl[MAX_TASK_NUMBER].sleep_time != 0 ) {
                    g_task_tbl[MAX_TASK_NUMBER].sleep_time = 0;
                    g_task_tbl[MAX_TASK_NUMBER].counter = g_task_tbl[MAX_TASK_NUMBER].budget;
                }
                else {
                    if( g_task_tbl[MAX_TASK_NUMBER].counter == 0 ) {
                        g_task_tbl[MAX_TASK_NUMBER].counter = g_task_tbl[MAX_TASK_NUMBER].budget;
                    }
                }
                if( current_id != MAX_TASK_NUMBER )
                    g_switch_counter++;
                return MAX_TASK_NUMBER;
            }

            for(i=0;i<MAX_TASK_NUMBER;i++) {
                p = &g_task_tbl[i];
                if( p->task_id == i && p->task_func_p && ( p->state != TASK_UNINTERRUPTIBLE )  )  {
                    if( p->sleep_time == 0 ) {
                        p->counter = p->budget;
                    }
                    continue;
                }
                p->counter = 0;
            }        
        }
    }
    if( current_id != next )
        g_switch_counter++;
    return next;
}

