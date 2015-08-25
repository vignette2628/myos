#include <system.h>
#include <printk.h>
#include <mem.h>
#include <console.h>
#include <tty.h>
#include <hd.h>

/*c ��ں���,��start32.sģ��retָ��ػص�����ʱ�Ѿ�Ϊ32 λ����ģʽ*/
void kernel_start(void)
{        
    /*text VGA��ʾģʽ�Ѿ���bios���úã���ʼ��ַ:0xB8000��һ���ֽ�����˵��Ҫ��ʾ���ַ�����һ�����ʾ��ɫ*/
    /*����Ļ��ɿո���ɫΪ:�ڵף���ɫǰ��ɫ*/
    (void)console_init();
    
    /*��ӡ�õĻ����ʼ����0*/
    (void)printk_init();
    (void)tty_init();

    /*Ӳ��chs������ʼ��*/
    (void)hd_chs_setup();
    
    /*start32.s�ж����idt����һ���ձ���:�����κ��ж��źŶ���û��һ����Ӧ���жϴ�����
        ��Ҫ����Щ�ж��źų�ʼ��һ��Ĭ�ϵ��жϴ�����default_handle()��
    */
    (void)trap_init();
    
    /*������ȳ�ʼ������ʼ��ÿ������Ķ�ջ
        ����8253��ʱ��оƬ����Ϊʱ���ж�Դ�����롣������ʱ���ж���Ӧ���жϴ�����timer_interrupt()
        ͬʱ����0x80���ж���Ӧ���жϴ�����system_call_interrupt()
    */
    (void)sched_init();     

    /*Ӳ���жϳ�ʼ��*/
    (void)hd_init();
    
    
    /*���պͷ��������õ�sk buffer��ʼ����rx ring��tx ring����sk buffer����*/
    (void)sk_buff_init();

    /*ע��l2~l4��Э�鴦����:arp ,udp ip ,icmp*/
    (void)rx_init();

    /*ģ��iret�жϻص����л��� start_task() �������в�ʹ���ж�*/
    move_to_start_task();   

    /*never return*/
    return ;
}

