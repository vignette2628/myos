#include <start32.h>
#include <sched.h>
#include <system.h>
#include <io.h>
#include <printk.h>
#include <mem.h>
#include <tty.h>
#include <console.h>
#include <hd.h>


//输入:dx寄存器:port, cx寄存器:nr, edi寄存器:buf, cx和di寄存器变化
//从port中读取nr个双字节到es:di指定的内存中
#define port_read(port,buf,nr) \
__asm__("cld;rep;insw"::"d" (port),"D" (buf),"c" (nr))

//将ds:si指定内存中的nr个双字节数据写到port中
#define port_write(port,buf,nr) \
__asm__("cld;rep;outsw"::"d" (port),"S" (buf),"c" (nr))


hd_info_t g_hd_info[MAX_HD_NUMBER];

void hd_chs_setup(void)
{
    int i;
    unsigned char *tmp = (unsigned char *)HD_INFO_ADDR;
    
    for(i=0;i<MAX_HD_NUMBER;i++) {
        memset((unsigned char *)&g_hd_info[i], 0x0, sizeof(hd_info_t));
    }
    /*获取磁盘CHS参数*/
    g_hd_info[0].cyl = *(unsigned short *)&tmp[0];
    g_hd_info[0].head = *(unsigned char *)&tmp[2];
    g_hd_info[0].sect = *(unsigned char *)&tmp[14];
    g_hd_info[0].wpcom = *(unsigned short *)&tmp[5];
    g_hd_info[0].ctl = *(unsigned char *)&tmp[8];
    g_hd_info[0].lzone = *(unsigned short *)&tmp[12];
    if(!(g_hd_info[0].cyl) || !(g_hd_info[0].head) || !(g_hd_info[0].sect)) {
        printk_string(0,0,"wrong hard-disk setup info, system pending.", DISP_FG_RED);
        while(1);
    }

    return;
}

void hd_init(void)
{
    set_intr_gate(0x2E,&hd_interrupt);
    outb_p(inb_p(0x21)&0xfb,0x21);//0xfb: 0b11111011,表示将第二个8259a连上
    outb(inb_p(0xA1)&0xbf,0xA1);//0xbf: 0b10111111,表示第2个8259a的第6个中断线上
}


void hd_interrupt_handler(void)
{
    printk("received a hd interrupt\n");
    read_blk_test();
}

void hd_read_test(void)
{
    unsigned int block,dev;
    unsigned int sec,head,cyl;
    unsigned int nsect;    
    register int port asm("dx");

    dev = 0; 
    block = 0;

    block = block / g_hd_info[0].sect;
    sec = block % g_hd_info[0].sect;

    cyl = block / g_hd_info[0].head;
    head = block % g_hd_info[0].head;

    sec++;
    nsect = 2; 

    outb_p(g_hd_info[0].ctl, HD_CMD);
    port = HD_DATA;
    outb_p(g_hd_info[0].wpcom>>2,++port);
    outb_p(nsect,++port);
    outb_p(sec,++port);
    outb_p(cyl,++port);
    outb_p(cyl>>8,++port);
    outb_p(0xA0|(dev<<4)|head,++port);
    outb(WIN_READ,++port);    
}
static int win_result(void)
{
	int i=inb_p(HD_STATUS);

	if ((i & (BUSY_STAT | READY_STAT | WRERR_STAT | SEEK_STAT | ERR_STAT))
		== (READY_STAT | SEEK_STAT))
		return(0); /* ok */
	if (i&1) i=inb(HD_ERROR);
	return (1);
}

char aaaa[512];
void read_blk_test(void)
{
    char *abc = (char *)&aaaa[0];
    int i;
    struct partition *p;
    
    memset(abc, 0x23, 512);
    if( win_result()) {
            printk("over\n");
            return;
    }

    port_read(HD_DATA,abc,256);
    rmb();
    wmb();
    for(i=500;i<512;i++)
        printk("%x ", aaaa[i]);
    printk("\n");

    p = 0x1be + (void *)abc;
    for(i=1;i<5;i++, p++) {
        printk("i:%d, start_sect:%d, nr_sects:%d\n", i, p->start_sect, p->nr_sects);
    }
}



