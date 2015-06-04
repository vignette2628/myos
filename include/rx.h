#ifndef __RX_H__

#define __RX_H__
#include <list.h>
#define MAC_LENGTH 6

struct packet_type {
    unsigned short	 type;	
    int (*func) (struct sk_buff *);
    struct list_head list;
};



struct ethhdr {
    unsigned char dest[MAC_LENGTH];	
    unsigned char	source[MAC_LENGTH];	
    unsigned short	 proto;		
} __attribute__((packed));


void rx_process(struct sk_buff *skb);
#endif
