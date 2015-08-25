#include <start32.h>
#include <sched.h>
#include <system.h>
#include <io.h>
#include <printk.h>
#include <pcnet32.h>
#include <mem.h>
#include <rx.h>
#include <arp.h>
#include <ip.h>

static struct list_head g_type_base[16];



void add_type(struct packet_type *pt)
{
    int hash;
    hash = ntohs(pt->type) & 15;
    
    list_add(&pt->list, &g_type_base[hash], g_type_base[hash].next );
    return;
}

/*ip*/
static struct packet_type ip_packet_type;
struct net_protocol_s *inet_protos[MAX_INET_PROTOS];

int inet_add_protocol(struct net_protocol_s *prot, unsigned char protocol)
{
	int hash, ret;

	hash = protocol & (MAX_INET_PROTOS - 1);

	if (inet_protos[hash]) {
		ret = -1;
	} else {
		inet_protos[hash] = prot;
		ret = 0;
	}

	return ret;
}

 unsigned short  inet_cksum(unsigned short *addr, int nleft)
{
	unsigned sum = 0;
	while (nleft > 1) {
		sum += *addr++;
		nleft -= 2;
	}

	if (nleft == 1) {
		sum += *(unsigned char *)addr;

	}

	sum = (sum >> 16) + (sum & 0xffff);     
	sum += (sum >> 16);                     

	return (unsigned short)~sum;
}

/*udp*/
int udp_rcv(struct sk_buff *skb)
{
//        printk("udp_recv()\n");
	return ;
}
static struct net_protocol_s g_udp_protocol = {
	.handler = udp_rcv,
};

void udp_init(void)
{
    inet_add_protocol(&g_udp_protocol, IPPROTO_UDP);
}

/*udp end*/

static int ip_rcv(struct sk_buff *skb)
{
    ip_hdr_t *iph = ip_hdr(skb);
    int hash;
    struct net_protocol_s *ipprot;
    
    /*not support fragment*/

    hash = iph->protocol & (MAX_INET_PROTOS - 1);

    ipprot = inet_protos[hash];
    if( ipprot ) {
        return ipprot->handler(skb);

    }

    return 0;
}
void ip_init(void)
{
    ip_packet_type.type =  htons(ETH_P_IP);
    ip_packet_type.func = ip_rcv;
    add_type(&ip_packet_type);
    g_ip_address = (0x0a000001); /*10.0.0.1*/

}

/*ip end*/

/**arp ***/

arp_table_t g_arp_table[ARP_TABLE_NUMBER];/*only support 16 mac record*/

static inline arphdr_t *arp_hdr(const struct sk_buff *skb)
{
    return (arphdr_t *)((unsigned char *)( skb->data + 14 ));
}

void arp_reply(unsigned char *target_mac, unsigned int target_ip)
{
    struct sk_buff *skb;
    struct arphdr_s *arphdr;
    unsigned char *up;
    unsigned char i;
    unsigned short *proto;
    unsigned int sender_ip;
    unsigned char *tmp;
    
    /*send arp reply*/
    skb = get_tx_skb();
    if( !skb ) {
        return;
    }
    /*fill Length of L2 layer head*/
    up = skb->data;
    for(i=0;i<6;i++) {
        up[i] = target_mac[i];
    }
    up += 6;
    for(i = 0; i<6;i++) {
        up[i] = g_mac_address[i];
    }
    up += 6;
    proto = (unsigned short *)&up[0];
    *proto = htons(ETH_P_ARP);

    arphdr = (struct arphdr_s *)arp_hdr(skb);
    arphdr->hardware_type = htons(1);
    arphdr->proto_type = htons(ETH_P_IP);
    arphdr->hardware_size = 6;
    arphdr->proto_size = 4;
    arphdr->op_code = htons(ARPOP_REPLY);


    up = (unsigned char *)(arphdr+1);

    for(i = 0; i < 6; i++) {
        up[i] = g_mac_address[i];
    }
    
    up += 6;    
    sender_ip = htonl(g_ip_address);
    tmp = (unsigned char *)&sender_ip;
    
    for(i=0;i<4;i++) {
        up[i] = tmp[i];
    }
   

    up += sizeof(sender_ip);
    for(i=0; i< 6; i++) {
        up[i] = target_mac[i];
    }

    up += 6;
    tmp = (unsigned char *)&target_ip;
    for(i=0;i<4;i++) {
        up[i] = tmp[i];
    }    

    skb->length = 34 + sizeof(struct arphdr_s );
    pcnet32_start_xmit(skb);    
    
}


void update_arp_table(unsigned char *mac, unsigned int ip)
{
    int i;
    
    if(!mac || ip == 0)
        return;
    for(i = 0; i< ARP_TABLE_NUMBER;i++){
        /*lock it later*/        
        if( g_arp_table[i].timeout == 0 ) {
            cli();
            g_arp_table[i].timeout = MAX_ARP_TIME_OUT;
            sti();
            memcpy(g_arp_table[i].mac, mac, 6 );
            g_arp_table[i].ip = ntohl(ip);

        }
    }
    /*need adder a timer to reduce timeout counter*/
}

static int arp_rcv(struct sk_buff *skb)
{
    arphdr_t *arphdr;
    u32 sender_ip, target_ip;
    u8 *sender_mac;
    u8 *up;

    if( skb->length < 60 ) {
        printk("Invalid length:%d\n", skb->length);
        return -1;
    }

    arphdr = (arphdr_t *)arp_hdr(skb); /*14: Length of L2 layer head */
    if( ( arphdr->hardware_size != sizeof(g_mac_address) ) || (arphdr->proto_size != 4) ) {
        printk("Invalid hardware size:%d or proto size:%d\n", arphdr->hardware_size, arphdr->proto_size);
        return -1;
    }

    if( arphdr->proto_type != htons(ETH_P_IP) ) {
        printk("Invalid proto type:%d\n", arphdr->proto_type);
        return -1;
    }
    /*only support arp request and reply*/
    if( arphdr->op_code != htons(ARPOP_REQUEST) && arphdr->op_code != htons(ARPOP_REPLY) ) {
        printk("Invalid arp op code:%d\n", arphdr->op_code);
        return -1;
    }

    up = (u8 *)(arphdr+1);
    sender_mac = up;

    up += sizeof(g_mac_address);
    memcpy(&sender_ip, up, sizeof(sender_ip));

    up+= sizeof(sender_ip);
    up+= sizeof(g_mac_address);

    memcpy(&target_ip, up, sizeof(target_ip) );

    if( g_ip_address == 0 || g_ip_address != ntohl(target_ip)) {
        /*not for me*/
        //printk("not for me with g_ip_address:%x, target_ip:%x\n", g_ip_address, target_ip);
        return 0;
    }    
    
    if( arphdr->op_code == htons(ARPOP_REQUEST) ){

        /*send arp reply*/
        (void)arp_reply(sender_mac, sender_ip  );
        (void)update_arp_table(sender_mac, sender_ip);
        return 0;
    }

    if( arphdr->op_code == htons(ARPOP_REPLY) ) {
        /*update arp table*/
        (void)update_arp_table(sender_mac, sender_ip);
        return 0;
    }
    
    return 0;
}



static struct packet_type arp_packet_type;


void arp_init(void)
{
    arp_packet_type.type =  htons(ETH_P_ARP);
    arp_packet_type.func = arp_rcv;
    add_type(&arp_packet_type);

    memset((unsigned char *)&g_arp_table[0], 0x0, sizeof(g_arp_table));
}
/***arp end***/


/*icmp*/
void icmp_reply(struct sk_buff *skb)
{
    struct sk_buff *new_skb;
    unsigned char i;
    struct ip_hdr_s *iph, *old_iph;
    struct icmp_hdr_s *icmphdr, *old_icmphdr;
    unsigned short left_len;
    unsigned char *up, *old_up;

    new_skb = get_tx_skb();
    if( !new_skb)
        return;
    

    /*L2 header*/
    for(i=0;i<6;i++) {
        new_skb->data[i+6] = skb->data[i];
        new_skb->data[i] = skb->data[i+6];
        new_skb->data[12] = skb->data[12];
        new_skb->data[13] = skb->data[13];
    }


    /*ip header*/
    iph = ip_hdr(new_skb);
    old_iph = ip_hdr(skb);
    //memcpy((unsigned char *)iph, (unsigned char *)old_iph, sizeof(struct ip_hdr_s));
    iph->ihl = 5;
    iph->version = 4;
    iph->tos = 0;
    iph->tot_len = htons( skb->length - 14 );
    iph->id = 0;
    iph->frag_off = 0;
    iph->ttl = 64;
    iph->protocol = IPPROTO_ICMP;
    iph->saddr = htonl(g_ip_address);
    iph->daddr = (old_iph->saddr);
    iph->check = 0x0;
    iph->check =  inet_cksum((unsigned short *)iph, sizeof(struct ip_hdr_s));

    icmphdr = icmp_hdr(new_skb);
    old_icmphdr = icmp_hdr(skb);
    icmphdr->type = ICMP_ECHOREPLY;
    icmphdr->code = 0;
    icmphdr->un = old_icmphdr->un;

    left_len = skb->length - 14 - 20 - sizeof(struct icmp_hdr_s);
    up = &new_skb->data[14+20 + sizeof(struct icmp_hdr_s)];
    old_up = &skb->data[14+20 + sizeof(struct icmp_hdr_s)];

        

    for(i=0; i < left_len; i++) {
        up[i] = old_up[i];
    //    if(i<8)
    //        printk("%x ", up[i]);
    }
//    printk("\n");



    new_skb->length = skb->length;

   // wmb();
    
    pcnet32_start_xmit(new_skb);
    
}

static void icmp_discard(struct sk_buff *skb)
{
    printk("icmp_discard()\n");
    return;
}
static const struct icmp_control_s g_icmp_control_tbl[NR_ICMP_TYPES+1] = {
	[ICMP_ECHOREPLY] = {
		.handler = icmp_discard,
                .error = 0,
	},
	[ICMP_ECHO] = {
		.handler = icmp_reply,
                .error = 0,
	},	
};


int icmp_rcv(struct sk_buff *skb)
{
    struct icmp_hdr_s *icmphdr;
    icmphdr = icmp_hdr(skb);
    g_icmp_control_tbl[icmphdr->type].handler(skb);

    
    return 0;
}

static struct net_protocol_s g_icmp_protocol = {
	.handler = icmp_rcv,
};

void icmp_init(void)
{
    inet_add_protocol(&g_icmp_protocol, IPPROTO_ICMP);
}

/*icmp end*/

static inline int is_multicast_ether_addr(const u8 *addr)
{
    return (0x01 & addr[0]);
}
static inline int is_zero_ether_addr(const u8 *addr)
{
    return !(addr[0] | addr[1] | addr[2] | addr[3] | addr[4] | addr[5]);
}

static inline int is_myself_ether_addr(const u8 *addr)
{
    int i;
    for(i=0; i< MAC_LENGTH;i++) {
        if(g_mac_address[i] != addr[i])
            return 0;
    }

    return 1;
}

static inline int is_broadcast_ether_addr(const u8 *addr)
{
    return (addr[0] & addr[1] & addr[2] & addr[3] & addr[4] & addr[5]) == 0xff;
}

static inline int is_valid_ether_addr(const u8 *addr)
{
	/* FF:FF:FF:FF:FF:FF is a multicast address so we don't need to
	 * explicitly check for it here. */
	return !is_multicast_ether_addr(addr) && !is_zero_ether_addr(addr);
}
void rx_process(struct sk_buff *skb)
{
    unsigned char found = 0;
    struct ethhdr *eth = (struct ethhdr *)(skb->data);
    unsigned short type;
    struct packet_type *ptype;
    int i;
    
    if(!skb) {
        return;

    }

    if(!is_valid_ether_addr(eth->source)) {
        printk("Invalid ether addr\n");
        return;
    }
    
    if( !is_broadcast_ether_addr(eth->dest) && !is_myself_ether_addr(eth->dest)) {
        //printk("is not for me\n");
        return;

    }

    type = eth->proto;
    list_for_each(ptype, &g_type_base[ntohs(type) & 15], list) {
        if( ptype->type == type ) {
            found = 1;
            break;
        }
    }

    if( !found ) {
        printk("not found with type:%d\n", type);
        return;
    }
    ptype->func(skb);

    return;
}

void rx_init(void)
{
    int i;
    for(i=0;i<16;i++)
        INIT_LIST_HEAD(&g_type_base[i]);
    arp_init();
    ip_init();
    udp_init();
    icmp_init();
}
