#include <system.h>
#include <printk.h>
#include <mem.h>
#include <console.h>
#include <tty.h>
#include <hd.h>

/*c 入口函数,由start32.s模仿ret指令返回回调，此时已经为32 位保护模式*/
void kernel_start(void)
{        
    /*text VGA显示模式已经由bios配置好，起始地址:0xB8000，一个字节用来说明要显示的字符，另一个则表示颜色*/
    /*把屏幕清成空格，颜色为:黑底，白色前景色*/
    (void)console_init();
    
    /*打印用的缓冲初始化成0*/
    (void)printk_init();
    (void)tty_init();

    /*硬盘chs参数初始化*/
    (void)hd_chs_setup();
    
    /*start32.s中定义的idt表是一个空表，即:触发任何中断信号都还没有一个相应的中断处理函数
        需要对这些中断信号初始化一个默认的中断处理函数default_handle()。
    */
    (void)trap_init();
    
    /*任务调度初始化，初始化每个任务的堆栈
        配置8253定时器芯片，作为时钟中断源的输入。并设置时钟中断相应的中断处理函数timer_interrupt()
        同时设置0x80软中断相应的中断处理函数system_call_interrupt()
    */
    (void)sched_init();     

    /*硬盘中断初始化*/
    (void)hd_init();
    
    
    /*接收和发送数据用的sk buffer初始化由rx ring和tx ring决定sk buffer数量*/
    (void)sk_buff_init();

    /*注册l2~l4层协议处理函数:arp ,udp ip ,icmp*/
    (void)rx_init();

    /*模仿iret中断回调，切换到 start_task() 函数运行并使能中断*/
    move_to_start_task();   

    /*never return*/
    return ;
}

