#include <system.h>
#include <pcnet32.h>
#include <io.h>
#include <pci_regs.h>
#include <sched.h>
#include <printk.h>
#include <mem.h>

int g_card_init_ok = -1;
unsigned int g_io_address;
unsigned char	g_mac_address[6]; /* permanent hw address */
unsigned int g_ip_address = 0;
unsigned int g_sub_mask = 0xffffff00;
struct pcnet32_access *g_pcnet32_access = NULL;
struct pcnet32_private g_lp;
unsigned int volatile g_tx_packets = 0;
unsigned int volatile g_rx_packets = 0;

/*borrow source code from linux source code: early.c */
unsigned int read_pci_config(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
    unsigned int v;
    outl(0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | offset, 0xcf8);
    v = inl(0xcfc);
    return v;
}

unsigned char read_pci_config_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
    unsigned char v;
    outl(0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | offset, 0xcf8);
    v = inb(0xcfc + (offset&3));
    return v;
}

unsigned short read_pci_config_16(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset)
{
    unsigned short v;
    outl(0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | offset, 0xcf8);
    v = inw(0xcfc + (offset&2));
    return v;
}

void write_pci_config(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset,
				    unsigned int val)
{
    outl(0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | offset, 0xcf8);
    outl(val, 0xcfc);
}

void write_pci_config_byte(unsigned char bus, unsigned char slot, unsigned char func, unsigned char offset, unsigned char val)
{
    outl(0x80000000 | (bus<<16) | (slot<<11) | (func<<8) | offset, 0xcf8);
    outb(val, 0xcfc);
}

/*borrow source code form linux source code: pcnet32.c*/
static unsigned short pcnet32_wio_read_csr(unsigned long addr, int index)
{
    outw(index, addr + PCNET32_WIO_RAP);
    return inw(addr + PCNET32_WIO_RDP);
}
static unsigned short pcnet32_dwio_read_csr(unsigned long addr, int index)
{
    outl(index, addr + PCNET32_DWIO_RAP);
    return (inl(addr + PCNET32_DWIO_RDP) & 0xffff);
}
static void pcnet32_wio_write_csr(unsigned long addr, int index, unsigned short val)
{
    outw(index, addr + PCNET32_WIO_RAP);
    outw(val, addr + PCNET32_WIO_RDP);
}
static void pcnet32_dwio_write_csr(unsigned long addr, int index, unsigned short val)
{
    outl(index, addr + PCNET32_DWIO_RAP);
    outl(val, addr + PCNET32_DWIO_RDP);
}

static unsigned short pcnet32_wio_read_bcr(unsigned long addr, int index)
{
    outw(index, addr + PCNET32_WIO_RAP);
    return inw(addr + PCNET32_WIO_BDP);
}


static unsigned short pcnet32_dwio_read_bcr(unsigned long addr, int index)
{
    outl(index, addr + PCNET32_DWIO_RAP);
    return (inl(addr + PCNET32_DWIO_BDP) & 0xffff);
}

static void pcnet32_wio_write_bcr(unsigned long addr, int index, unsigned short val)
{
    outw(index, addr + PCNET32_WIO_RAP);
    outw(val, addr + PCNET32_WIO_BDP);
}

static void pcnet32_dwio_write_bcr(unsigned long addr, int index, unsigned short val)
{
    outl(index, addr + PCNET32_DWIO_RAP);
    outl(val, addr + PCNET32_DWIO_BDP);
}

static unsigned short pcnet32_wio_read_rap(unsigned long addr)
{
    return inw(addr + PCNET32_WIO_RAP);
}

static unsigned short pcnet32_dwio_read_rap(unsigned long addr)
{
    return (inl(addr + PCNET32_DWIO_RAP) & 0xffff);
}

static void pcnet32_wio_write_rap(unsigned long addr, unsigned short val)
{
    outw(val, addr + PCNET32_WIO_RAP);
}

static void pcnet32_dwio_write_rap(unsigned long addr, unsigned short val)
{
    outl(val, addr + PCNET32_DWIO_RAP);
}

static void pcnet32_wio_reset(unsigned long addr)
{
    inw(addr + PCNET32_WIO_RESET);
}
static void pcnet32_dwio_reset(unsigned long addr)
{
    inl(addr + PCNET32_DWIO_RESET);
}

static int pcnet32_wio_check(unsigned long addr)
{
    outw(88, addr + PCNET32_WIO_RAP);
    return (inw(addr + PCNET32_WIO_RAP) == 88);
}
static int pcnet32_dwio_check(unsigned long addr)
{
    outl(88, addr + PCNET32_DWIO_RAP);
    return ((inl(addr + PCNET32_DWIO_RAP) & 0xffff) == 88);
}

static struct pcnet32_access g_pcnet32_dwio = {
	.read_csr = pcnet32_dwio_read_csr,
	.write_csr = pcnet32_dwio_write_csr,
	.read_bcr = pcnet32_dwio_read_bcr,
	.write_bcr = pcnet32_dwio_write_bcr,
	.read_rap = pcnet32_dwio_read_rap,
	.write_rap = pcnet32_dwio_write_rap,
	.reset = pcnet32_dwio_reset
};

static struct pcnet32_access g_pcnet32_wio = {
	.read_csr = pcnet32_wio_read_csr,
	.write_csr = pcnet32_wio_write_csr,
	.read_bcr = pcnet32_wio_read_bcr,
	.write_bcr = pcnet32_wio_write_bcr,
	.read_rap = pcnet32_wio_read_rap,
	.write_rap = pcnet32_wio_write_rap,
	.reset = pcnet32_wio_reset
};



static void pcnet32_alloc_ring(struct pcnet32_private *lp)
{
    lp->tx_ring = (struct pcnet32_tx_head *)MEM_TX_HEADER_ADDR;
    lp->tx_ring_dma_addr = MEM_TX_HEADER_ADDR;
    memset((u8 *)lp->tx_ring, 0x0, (lp->tx_ring_size * sizeof(struct pcnet32_tx_head)) );
    
    lp->rx_ring = (struct pcnet32_rx_head *)MEM_RX_HEADER_ADDR;
    lp->rx_ring_dma_addr = MEM_RX_HEADER_ADDR;
    memset((u8 *)lp->rx_ring, 0x0, (lp->rx_ring_size * sizeof(struct pcnet32_rx_head)) );
    
    lp->tx_dma_addr = (unsigned int *)MEM_TX_DMA_ADDR;
    memset((u8 *)lp->tx_dma_addr, 0x0, ( sizeof(dma_addr_t) * lp->tx_ring_size ) );
    lp->rx_dma_addr = (unsigned int *)MEM_RX_DMA_ADDR;
    memset((u8 *)lp->rx_dma_addr, 0x0, ( sizeof(dma_addr_t) * lp->rx_ring_size ) );
    
    lp->tx_skbuff = (struct sk_buff **)MEM_TX_BUF_ADDR;
    memset((u8 *)lp->tx_skbuff, 0x0, (lp->tx_ring_size * sizeof(struct sk_buff *)) );
    lp->rx_skbuff = (struct sk_buff **)MEM_RX_BUF_ADDR;
    memset((u8 *)lp->rx_skbuff, 0x0, (lp->rx_ring_size * sizeof(struct sk_buff *)) );
    
}
static int pcnet32_init_ring(void)
{
    struct pcnet32_private *lp =&g_lp;
    int i;

    lp->tx_full = 0;
    lp->cur_rx = lp->cur_tx = 0;
    lp->dirty_rx = lp->dirty_tx = 0;

    for (i = 0; i < lp->rx_ring_size; i++) {
        struct sk_buff *rx_skbuff = lp->rx_skbuff[i];
        rx_skbuff = lp->rx_skbuff[i] = (struct sk_buff *)((unsigned char *)(MEM_PCNET32_RX_SK_BUFF_BASE_ADDR+sizeof(struct sk_buff)*i));
        rx_skbuff->data = (unsigned char *)(MEM_PCNET32_RX_SK_BUFF_DATA_BASE_ADDR+PAGE_SIZE*i);
        rx_skbuff->data += (NET_SKB_PAD+2);

        rmb(); 
        lp->rx_dma_addr[i] = (unsigned int)rx_skbuff->data;
        lp->rx_ring[i].base = (unsigned int) (lp->rx_dma_addr[i] );
        lp->rx_ring[i].buf_length = (2 - PKT_BUF_SZ);
        wmb(); 		/* Make sure owner changes after all others are visible */
        lp->rx_ring[i].status = (0x8000);
    }

    for (i = 0; i < lp->tx_ring_size; i++) {
        lp->tx_ring[i].status = 0;	/* CPU owns buffer */
        wmb();		/* Make sure adapter sees owner change */
        lp->tx_ring[i].base = 0;
        lp->tx_dma_addr[i] = 0;
    }
    
    lp->init_block->tlen_rlen = (lp->tx_len_bits | lp->rx_len_bits);
    for (i = 0; i < 6; i++)
        lp->init_block->phys_addr[i] = g_mac_address[i];
    lp->init_block->rx_ring = (unsigned int) (lp->rx_ring_dma_addr);
    lp->init_block->tx_ring = (unsigned int) (lp->tx_ring_dma_addr);
    wmb(); 			/* Make sure all changes are visible */
    
    return 0;

}

static void pcnet32_load_multicast(void)
{
    struct pcnet32_private *lp = &g_lp;
    volatile struct pcnet32_init_block *ib = lp->init_block;
    unsigned long ioaddr = g_io_address;

    /* set all multicast bits */
    ib->filter[0] = 0xffffffff;
    ib->filter[1] = 0xffffffff;
    g_pcnet32_access->write_csr(ioaddr, PCNET32_MC_FILTER, 0xffff);
    g_pcnet32_access->write_csr(ioaddr, PCNET32_MC_FILTER+1, 0xffff);
    g_pcnet32_access->write_csr(ioaddr, PCNET32_MC_FILTER+2, 0xffff);
    g_pcnet32_access->write_csr(ioaddr, PCNET32_MC_FILTER+3, 0xffff);
    return;
}

int pcnet32_open(unsigned int ioaddr)
{
    u16 val;
    struct pcnet32_private *lp = &g_lp;
    int i;
    u8 irq;
    

    //printk("open pcnet32 card...");
    g_pcnet32_access->reset(ioaddr);
    g_pcnet32_access->write_bcr(ioaddr, 20, 2);
    
    val = g_pcnet32_access->read_bcr(ioaddr, 2) & ~2;
    val |= 2; /*auto select*/
    g_pcnet32_access->write_bcr(ioaddr, 2, val);

    val = g_pcnet32_access->read_bcr(ioaddr, 9) & ~3;
    g_pcnet32_access->write_bcr(ioaddr, 9, val);

    /* set/reset GPSI bit in test register */
    val = g_pcnet32_access->read_csr(ioaddr, 124) & ~0x10;
    g_pcnet32_access->write_csr(ioaddr, 124, val);

    g_pcnet32_access->write_bcr(ioaddr, 32, g_pcnet32_access->read_bcr(ioaddr,32) | 0x0080);
    /* enable auto negotiate, setup, disable fd */
    val = g_pcnet32_access->read_bcr(ioaddr, 32) & ~0x98;
    val |= 0x20;
    g_pcnet32_access->write_bcr(ioaddr, 32, val);

    lp->init_block->mode =0;
    pcnet32_load_multicast();

    pcnet32_init_ring();

    /* Re-initialize the PCNET32, and start it when done. */
    g_pcnet32_access->write_csr(ioaddr, 1, (lp->init_dma_addr & 0xffff));
    g_pcnet32_access->write_csr(ioaddr, 2, (lp->init_dma_addr >> 16));

    g_pcnet32_access->write_csr(ioaddr, CSR4, 0x0915);	/* auto tx pad */
    g_pcnet32_access->write_csr(ioaddr, CSR0, CSR0_INIT);    

    i = 0;
    while (i++ < 100)
	if (g_pcnet32_access->read_csr(ioaddr, CSR0) & CSR0_IDON)
		break;
    
    //printk("Done[%d]\n", i);
    
    g_pcnet32_access->write_csr(ioaddr, CSR0, CSR0_NORMAL);

    return 0;
    
}

static int pcnet32_prob(unsigned int ioaddr)
{
    struct pcnet32_private *lp = &g_lp;
    char *chipname;
    int chip_version;
    unsigned int i;
    pcnet32_wio_reset(ioaddr);
    if (pcnet32_wio_read_csr(ioaddr, 0) == 4 && pcnet32_wio_check(ioaddr)) {
        //printk("Using wio mode\n");
        g_pcnet32_access = &g_pcnet32_wio;
        
    }
    else {
        pcnet32_dwio_reset(ioaddr);
        if (pcnet32_dwio_read_csr(ioaddr, 0) == 4 && pcnet32_dwio_check(ioaddr)) {
            g_pcnet32_access = &g_pcnet32_dwio;
            //printk("Using dwio mode\n");
        } else {
            printk("Unknown io mode, pcnet32_prob() failed\n");
            return -1;
        }
    }
    chip_version = g_pcnet32_access->read_csr(ioaddr, 88) | (g_pcnet32_access->read_csr(ioaddr, 89) << 16);
    if ((chip_version & 0xfff) != 0x003) {
        printk("Invalid chip_version:%x\n", chip_version);
        return -1;
    }
    chip_version = (chip_version >> 12) & 0xffff;
    switch (chip_version) {
        case 0x2621:
            chipname = "PCnet/PCI II 79C970A";
            break;
        default:
            chipname = "Unknow";
            printk("unknown chip version:%x\n", chip_version);
            return -1;
    }

    for( i =0; i < 3; i++) {
        unsigned int val;
        val = g_pcnet32_access->read_csr(ioaddr, i + 12) & 0x0ffff;
        g_mac_address[2*i] = val & 0x0ff;
        g_mac_address[2 * i + 1] = (val >> 8) & 0x0ff;
    }
    
    /*init init_block*/
    lp->init_block = (struct pcnet32_init_block *)MEM_BLOCK_INIT_HEADER;
    lp->init_dma_addr = (dma_addr_t)MEM_BLOCK_INIT_HEADER;
    memset((u8 *)lp->init_block, 0x0, sizeof(struct pcnet32_init_block));
    
    lp->tx_ring_size = TX_RING_SIZE;	/* default tx ring size:16 */
    lp->rx_ring_size = RX_RING_SIZE;	/* default rx ring size:32 */
    lp->tx_mod_mask = lp->tx_ring_size - 1;
    lp->rx_mod_mask = lp->rx_ring_size - 1;
    lp->tx_len_bits = (PCNET32_LOG_TX_BUFFERS << 12);
    lp->rx_len_bits = (PCNET32_LOG_RX_BUFFERS << 4);
    //lp->a = g_pcnet32_access;
    (void )pcnet32_alloc_ring(lp);

    lp->init_block->mode = (0x0003);	/* Disable Rx and Tx. */
    lp->init_block->tlen_rlen = (lp->tx_len_bits | lp->rx_len_bits);
    for (i = 0; i < 6; i++)
    	lp->init_block->phys_addr[i] = g_mac_address[i];
    lp->init_block->filter[0] = 0x00000000;
    lp->init_block->filter[1] = 0x00000000;
    lp->init_block->rx_ring = (unsigned int) (lp->rx_ring_dma_addr);
    lp->init_block->tx_ring = (unsigned int) (lp->tx_ring_dma_addr);


    /* switch pcnet32 to 32bit mode */
    g_pcnet32_access->write_bcr(ioaddr, 20, 2);

    g_pcnet32_access->write_csr(ioaddr, 1, (lp->init_dma_addr & 0xffff));
    g_pcnet32_access->write_csr(ioaddr, 2, (lp->init_dma_addr >> 16));

    /* enable LED writes */
    g_pcnet32_access->write_bcr(ioaddr, 2, g_pcnet32_access->read_bcr(ioaddr, 2) | 0x1000);


    
    return 0;
    
}

unsigned char pcnet32_get_interrupt_pin(void)
{
    unsigned char irq_line;
    irq_line = read_pci_config_byte(0, 3, 0, PCI_INTERRUPT_PIN);
    return irq_line;
}
void pcnet32_rx_entry(struct pcnet32_private *lp, struct pcnet32_rx_head *rxp, int entry)
{
    int i;
    int status = (short)(rxp->status) >> 8;
    unsigned short pkt_len;
    struct sk_buff *skb;
    struct sk_buff *new_skb;
    
    if( status != 0x03) {
        printk("failed to received packet\n");
        return;
    }

    pkt_len = ((rxp->msg_length) & 0xfff) -4;

    if( pkt_len > (PKT_BUF_SZ -2) || pkt_len < 60 ) {
        printk("Invalid packet size:%d\n", pkt_len);
        return;
    }

    skb = lp->rx_skbuff[entry];
    skb->length = pkt_len;
    put_rx_skb(skb);


}

int pcnet32_rx(struct pcnet32_private *lp, int quota)
{
    int entry = lp->cur_rx & lp->rx_mod_mask;
    struct pcnet32_rx_head *rxp = &lp->rx_ring[entry];
    int npackets = 0;
    char tmp_buf[64];
    //static int total_pkts = 0;
   
    /* If we own the next entry, it's a new packet. Send it up. */
    while (npackets < quota && (short)(rxp->status) >= 0) {
        pcnet32_rx_entry( &g_lp, rxp, entry);
        npackets += 1;
        /*
        * The docs say that the buffer length isn't touched, but Andrew
        * Boyd of QNX reports that some revs of the 79C965 clear it.
        */
        rxp->buf_length = (2 - PKT_BUF_SZ);
        wmb();	/* Make sure owner changes after others are visible */
        rxp->status = (0x8000);
        entry = (++lp->cur_rx) & lp->rx_mod_mask;
        rxp = &lp->rx_ring[entry];
    }
    //total_pkts += npackets;
    g_rx_packets += npackets;
    //snprintf(tmp_buf, 32, "received %d packets", total_pkts );
    //printk_string(0, 3, tmp_buf, DISP_FG_RED );
    
    return npackets;
}
int pcnet32_start_xmit(struct sk_buff *skb )
{
    struct pcnet32_private *lp = &g_lp;
    unsigned int ioaddr = g_io_address;
    int entry;
    unsigned long flags;

    asm volatile("cli": : :"memory");

    /* Mask to ring buffer boundary. */
    entry = lp->cur_tx & lp->tx_mod_mask;
    
    /* Caution: the write order is important here, set the status
     * with the "ownership" bits last. */
    lp->tx_ring[entry].length = (-(skb->length));

    lp->tx_ring[entry].misc = 0x00000000;

    lp->tx_skbuff[entry] = skb;
    lp->tx_dma_addr[entry] = (dma_addr_t )skb->data;
    lp->tx_ring[entry].base = (u32) (lp->tx_dma_addr[entry]);
    wmb();			/* Make sure owner changes after all others are visible */
    lp->tx_ring[entry].status = (0x8300);

    lp->cur_tx++;

    /* Trigger an immediate send poll. */
    g_pcnet32_access->write_csr(ioaddr, CSR0, CSR0_INTEN | CSR0_TXPOLL);


    if (lp->tx_ring[(entry + 1) & lp->tx_mod_mask].base != 0) {
    	lp->tx_full = 1;
        printk("fulled\n");
    }
    asm volatile("sti": : :"memory");
    return 0;
}
static int pcnet32_tx(struct pcnet32_private *lp)
{
    unsigned int dirty_tx = lp->dirty_tx;
    int delta;
    int must_restart = 0;

    while (dirty_tx != lp->cur_tx) {
        int entry = dirty_tx & lp->tx_mod_mask;
        int status = (short)(lp->tx_ring[entry].status);
        
        if (status < 0) {
            printk("not txed:%x\n", status);
            break;	/* It still hasn't been Txed */

        }
        
        lp->tx_ring[entry].base = 0;
        if (status & 0x4000) {
            int err_status = (lp->tx_ring[entry].misc);
            if (err_status & 0x40000000) {
                printk("need restart pcnet32 chip\n");
                must_restart = 1;
            }
        }

        /* We must free the original skb */
		if (lp->tx_skbuff[entry]) {
		        put_tx_skb();   
			lp->tx_skbuff[entry] = NULL;
			lp->tx_dma_addr[entry] = 0;
                        g_tx_packets++;
		}
		dirty_tx++;        
    }

    delta = (lp->cur_tx - dirty_tx) & (lp->tx_mod_mask + lp->tx_ring_size);
    if (delta > lp->tx_ring_size) {
        dirty_tx += lp->tx_ring_size;
        delta -= lp->tx_ring_size;
    }

    if (lp->tx_full && delta < lp->tx_ring_size - 2) {
        /* The ring is no longer full, clear tbusy. */
        lp->tx_full = 0;
    }
    
    lp->dirty_tx = dirty_tx;

    return must_restart;
}
extern long volatile jiffies;
void pcnet32_interrupt(void)
{
    u16 csr0;
    unsigned int  ioaddr = g_io_address;
    int boguscnt = 2;
    u16 val;
    int work_done;
    
    csr0 = g_pcnet32_access->read_csr(ioaddr, CSR0);
    while ((csr0 & 0x8f00) && --boguscnt >= 0) {
        if (csr0 == 0xffff) {
            break;	/* PCMCIA remove happened */
        }
        if (csr0 & 0x1000) {
 
        }
        
        /* Acknowledge all of the current interrupt sources ASAP. */
        g_pcnet32_access->write_csr(ioaddr, CSR0, csr0 & ~0x004f);
        work_done = pcnet32_rx(&g_lp, ( g_lp.rx_ring_size/2));
        //printk("rx:%x\n", jiffies);
        if( pcnet32_tx(&g_lp)) {
            /*reset pcnet32 chip*/
        }        
        csr0 = g_pcnet32_access->read_csr(ioaddr, CSR0);
        
    }

    g_pcnet32_access->write_csr(ioaddr,  CSR0, CSR0_INTEN);

    outb_p(0x20, 0x20);
    outb_p(0x20, 0xa0);

    
    return;
}

void pcnet32_drivers_init(void)
{ 
    /*read pcie configuration sapce from AM79C970A chip */
    /*the command:"lspci -xxx" told me pcie message: 00:03.0 Ethernet controller: Advanced Micro Devices [AMD] 79c970 [PCnet32 LANCE] (rev 10)*/
    g_io_address = read_pci_config( 0, 3, 0, PCI_BASE_ADDRESS_0 );
    printk("find pcie device with io address:%x\n", g_io_address);
    if( g_io_address == 0xffffffff){
        printk("warning: no pcie device to be found!\n");
        //while(1);
        return;
    }
    
    g_io_address &= 0xfffffff0; 
    g_card_init_ok = pcnet32_prob(g_io_address);

    return;
}

unsigned char get_int_line(void)
{
    return read_pci_config_byte(0, 3, 0, PCI_INTERRUPT_LINE);
}

unsigned char get_int_pin(void)
{
    return read_pci_config_byte(0, 3, 0, PCI_INTERRUPT_PIN);
}
