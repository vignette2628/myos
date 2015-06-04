#include <system.h>
#include <mem.h>
#include <sched.h>

void * memcpy(void *dest, const void *src, u32 n)
{
    __asm__("cld\n\t"
           "rep\n\t" 
        "movsb" 
        : 
        :"c" (n), "S" (src), "D" (dest) 
        );
    return dest;
}

static int *g_rx_addr = (int *)MEM_RX_CONTROL_ADDR;
static int *g_tx_addr = (int *)MEM_TX_CONTROL_ADDR;

static int g_rx_head = 0;
static int g_rx_tail = 0;
static int g_tx_head = 0;
static int g_tx_tail = 0;

void sk_buff_init(void)
{
    int i;
    struct sk_buff *skb;
    struct list_head *list;
    
    for(i=0;i<SK_BUFFER_NUM;i++) {
        skb = (struct sk_buff *)((unsigned char *)MEM_RX_SK_BUFF_POOL_BASE_ADDR + (sizeof(struct sk_buff) *i));
        memset((unsigned char*)skb, 0x0, sizeof(struct sk_buff) );
        skb->data = (unsigned char *)((unsigned char *)MEM_RX_SK_BUFF_DATA_POOL_BASE_ADDR + (PAGE_SIZE*i));
        g_rx_addr[i] = (int)skb;
    
        skb = (struct sk_buff *)((unsigned char *)MEM_TX_SK_BUFF_POOL_BASE_ADDR + (sizeof(struct sk_buff) *i));
        memset((unsigned char*)skb, 0x0, sizeof(struct sk_buff) );
        skb->data = (unsigned char *)((unsigned char *)MEM_TX_SK_BUFF_DATA_POOL_BASE_ADDR + (PAGE_SIZE*i));
        skb->data += NET_SKB_PAD;
        g_tx_addr[i] = (int)skb;
    }
 
}
struct sk_buff *get_rx_skb(void)
{
    struct sk_buff *skb;
    
    if (g_rx_tail == g_rx_head )
    {
        return NULL;
    }
    g_rx_tail = (g_rx_tail+1)%SK_BUFFER_NUM;
    skb = (struct sk_buff *)g_rx_addr[g_rx_tail];

    return skb;
}

void put_rx_skb(struct sk_buff *skb)
{
    struct sk_buff *tmp_skb;
    int tmp_index = (g_rx_head+1)%SK_BUFFER_NUM;
    int i;
    
    if( !skb )
        return;

    if( tmp_index  == g_rx_tail) {
        return;
    }
    
    tmp_skb = (struct sk_buff *)g_rx_addr[tmp_index];
    memcpy(tmp_skb->data, skb->data, skb->length );
    tmp_skb->length = skb->length;
    g_rx_head = tmp_index;
    return;
}

void put_tx_skb(void)
{
    
    if (g_tx_tail == g_tx_head )
    {
        return;
    }
    g_tx_tail = (g_tx_tail+1)%SK_BUFFER_NUM;
    return;
}

struct sk_buff *get_tx_skb(void)
{
    struct sk_buff *skb;
    int tmp_index = (g_tx_head+1)%SK_BUFFER_NUM;
    int i;

    if( tmp_index  == g_tx_tail) {
        return NULL;
    }
    
    skb = (struct sk_buff *)g_tx_addr[tmp_index];
    
    g_tx_head = tmp_index;
    return skb;
}