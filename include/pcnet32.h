#ifndef __PCNET32_H__
#define __PCNET32_H__

/*copy from linux 2.6.22 source code: pcnet32.c*/


#define PCNET32_LOG_TX_BUFFERS		4
#define PCNET32_LOG_RX_BUFFERS		5
#define PCNET32_LOG_MAX_TX_BUFFERS	9	/* 2^9 == 512 */
#define PCNET32_LOG_MAX_RX_BUFFERS	9

#define TX_RING_SIZE		(1 << (PCNET32_LOG_TX_BUFFERS))
#define TX_MAX_RING_SIZE	(1 << (PCNET32_LOG_MAX_TX_BUFFERS))

#define RX_RING_SIZE		(1 << (PCNET32_LOG_RX_BUFFERS))
#define RX_MAX_RING_SIZE	(1 << (PCNET32_LOG_MAX_RX_BUFFERS))

#define PKT_BUF_SZ		1544


#define PKT_BUDGET 16


/* Offsets from base I/O address. */
#define PCNET32_WIO_RDP		0x10
#define PCNET32_WIO_RAP		0x12
#define PCNET32_WIO_RESET	0x14
#define PCNET32_WIO_BDP		0x16

#define PCNET32_DWIO_RDP	0x10
#define PCNET32_DWIO_RAP	0x14
#define PCNET32_DWIO_RESET	0x18
#define PCNET32_DWIO_BDP	0x1C


#define CSR0		0
#define CSR0_INIT	0x1
#define CSR0_START	0x2
#define CSR0_STOP	0x4
#define CSR0_TXPOLL	0x8
#define CSR0_INTEN	0x40
#define CSR0_IDON	0x0100
#define CSR0_NORMAL	(CSR0_START | CSR0_INTEN)
#define PCNET32_INIT_LOW	1
#define PCNET32_INIT_HIGH	2
#define CSR3		3
#define CSR4		4
#define CSR5		5
#define CSR5_SUSPEND	0x0001
#define CSR15		15
#define PCNET32_MC_FILTER	8

/* The PCNET32 Rx and Tx ring descriptors. */
struct pcnet32_rx_head {
	u32	base;
	s16	buf_length;	/* two`s complement of length */
	s16	status;
	u32	msg_length;
	u32	reserved;
};

struct pcnet32_tx_head {
	u32	base;
	s16	length;		/* two`s complement of length */
	s16	status;
	u32	misc;
	u32	reserved;
};

/* The PCNET32 32-Bit initialization block, described in databook. */
struct pcnet32_init_block {
	u16	mode;
	u16	tlen_rlen;
	u8	phys_addr[6];
	u16	reserved;
	u32	filter[2];
	/* Receive and transmit ring base, along with extra bits. */
	u32	rx_ring;
	u32	tx_ring;
};

/* PCnet32 access functions */
struct pcnet32_access {
	unsigned short	(*read_csr) (unsigned long, int);
	void	(*write_csr) (unsigned long, int, unsigned short);
	unsigned short	(*read_bcr) (unsigned long, int);
	void	(*write_bcr) (unsigned long, int, unsigned short);
	unsigned short	(*read_rap) (unsigned long);
	void	(*write_rap) (unsigned long, unsigned short);
	void	(*reset) (unsigned long);
};

extern struct pcnet32_access *g_pcnet32_access;
extern int g_card_init_ok;
extern unsigned int g_io_address;
extern unsigned int volatile g_tx_packets;
extern unsigned int volatile g_rx_packets;
/*
 * The first field of pcnet32_private is read by the ethernet device
 * so the structure should be allocated using pci_alloc_consistent().
 */
struct pcnet32_private {
	struct pcnet32_init_block *init_block;
	/* The Tx and Rx ring entries must be aligned on 16-byte boundaries in 32bit mode. */
	struct pcnet32_rx_head	*rx_ring;
	struct pcnet32_tx_head	*tx_ring;
	dma_addr_t		init_dma_addr;/* DMA address of beginning of the init block,
				   returned by pci_alloc_consistent */
	//void		*pci_dev;
	const char		*name;
	/* The saved address of a sent-in-place packet/buffer, for skfree(). */
	struct sk_buff		**tx_skbuff;
	struct sk_buff		**rx_skbuff;
	dma_addr_t		*tx_dma_addr;
	dma_addr_t		*rx_dma_addr;
	//struct pcnet32_access	*a;
	//spinlock_t		lock;		/* Guard lock */
	unsigned int		cur_rx, cur_tx;	/* The next free ring entry */
	unsigned int		rx_ring_size;	/* current rx ring size */
	unsigned int		tx_ring_size;	/* current tx ring size */
	unsigned int		rx_mod_mask;	/* rx ring modular mask */
	unsigned int		tx_mod_mask;	/* tx ring modular mask */
	unsigned short		rx_len_bits;
	unsigned short		tx_len_bits;
	dma_addr_t		rx_ring_dma_addr;
	dma_addr_t		tx_ring_dma_addr;
	unsigned int		dirty_rx,	/* ring entries to be freed. */
				dirty_tx;

	//struct net_device_stats	stats;
	char			tx_full;
	//char			phycount;	/* number of phys found */
	//int			options;
	//unsigned int		shared_irq:1,	/* shared irq possible */
	//			dxsuflo:1,   /* disable transmit stop on uflo */
	//			mii:1;		/* mii port available */
	//struct net_device	*next;
	//struct mii_if_info	mii_if;
	//struct timer_list	watchdog_timer;
	//struct timer_list	blink_timer;
	//u32			msg_enable;	/* debug message level */

	/* each bit indicates an available PHY */
	//u32			phymask;
	//unsigned short		chip_version;	/* which variant this is */
};
void pcnet32_drivers_init(void);
int pcnet32_open(unsigned int ioaddr);
unsigned char get_int_line(void);
unsigned char get_int_pin(void);
extern unsigned char g_mac_address[6];
extern unsigned int g_ip_address;
#endif
