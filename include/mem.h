#ifndef __MEM_H__

#define __MEM_H__

#define memory_size_address  (0x90200 + 1018 )
#define SK_BUFFER_NUM 64
#define PAGE_SIZE 4096
#define NET_SKB_PAD	16


/*memory MAP*/
enum {
        /*for printk*/
    MEM_PRINTK_BUF = 0x100000,
    MEM_COMMAND_LINE_BUF = 0x110000,
    
    MEM_BASE_ADDRESS = 0x200000, /*Begin from 2M*/
        
    /* for NIC card */
    MEM_BLOCK_INIT_HEADER = 0x200000, /*must page align*/

    MEM_TX_HEADER_ADDR = 0x210000, /*must page align*/
    MEM_RX_HEADER_ADDR = 0x220000, /*must page align*/

    MEM_TX_DMA_ADDR = 0x230000,
    MEM_RX_DMA_ADDR = 0x240000,

    MEM_TX_BUF_ADDR = 0x250000,
    MEM_RX_BUF_ADDR = 0x260000,

    MEM_PCNET32_RX_SK_BUFF_BASE_ADDR = 0x300000,   
    MEM_PCNET32_RX_SK_BUFF_DATA_BASE_ADDR = 0x400000,

    MEM_RX_CONTROL_ADDR = 0x500000,
    /*rx sk buffer*/
    MEM_RX_SK_BUFF_POOL_BASE_ADDR = 0x600000, 
    MEM_RX_SK_BUFF_DATA_POOL_BASE_ADDR = 0x800000,

    /*tx sk buffer*/
    MEM_TX_CONTROL_ADDR = 0xe00000,
    MEM_TX_SK_BUFF_POOL_BASE_ADDR = 0xf00000,
    MEM_TX_SK_BUFF_DATA_POOL_BASE_ADDR = 0x1100000,

    
    MEM_MAX_ADDRESS = 0x4000000,/*support max 64Mbytes*/
};
typedef struct queue_s {
    struct queue_s *next;
}queue_t;
struct sk_buff {
    struct queue_s queue;
    unsigned short length;
    unsigned short filler;
    unsigned char *data;
};

int mem_init(void);


static inline void memset(unsigned char *p, unsigned char v, unsigned int len )
{
    int i;
    
    if( !p )
        return;
    for( i = 0; i < len; i++ ) {
        p[i] = v;
    }

    return;
}
struct sk_buff *get_rx_skb(void);
void update_consume_index(void);
struct sk_buff *get_tx_skb(void);
#endif
