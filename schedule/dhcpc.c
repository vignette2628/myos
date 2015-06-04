#include <start32.h>
#include <sched.h>
#include <system.h>
#include <io.h>
#include <printk.h>
#include <pcnet32.h>
#include <mem.h>
#include <rx.h>
#include <dhcpc.h>
#include <ip.h>

const struct dhcp_opt_flag dhcp_optflags[] = {
        {DHCP_OPTION_53, DHCP_OPTION_53_LENGTH },
        {DHCP_OPTION_51, DHCP_OPTION_51_LENGTH },
        {DHCP_OPTION_55,  DHCP_OPTION_55_LENGTH},
        {DHCP_OPTION_255, 0}
};

static void fill_options(unsigned char *up, unsigned char option, unsigned char type)
{
    int *lease_time = (int *)up;
        
    switch (option) {
        case DHCP_OPTION_53:
            up[0] = type;
            break;
        case DHCP_OPTION_51:
            *lease_time = htonl(3600);
            
            break;

        case DHCP_OPTION_55:
            up[0] = 1;
            up[1] = 28;
            up[2] = 3;
            up[3] = 26;
            up[4] = 12;
            up[5] = 15;
            up[6] = 6;
            up[7] = 40;
            up[8] = 41;
            up[9] = 87;
            up[10] = 85;
            up[11] = 86;
            up[12] = 44;
            up[13] = 45;
            up[14] = 46;
            up[15] = 47;
            up[16] = 42;


            break;

         default:
            break;
    }
}
static void dhcp_construct_l2_header(struct sk_buff *skb, unsigned char *src_mac, unsigned char *dest_mac)
{
    int i;
    memcpy(&skb->data[0], dest_mac, 6);
    memcpy(&skb->data[6], src_mac, 6 );
    skb->data[12] = 0x08;
    skb->data[13] = 0x0;
}
static void prepare_bootp(struct sk_buff *skb, unsigned char message_type)
{
    int i;
    int offset = 0;
    struct dhcp_packet *dp = (struct dhcp_packet *)( skb->data + 14 + sizeof(struct ip_hdr_s) + sizeof(struct udp_hdr_s ));
    unsigned char *up = (unsigned char *)&(dp->options[0]);
    
    memset((unsigned char *)dp, 0x0, sizeof(struct dhcp_packet));
    dp->op = message_type;
    dp->htype = 1;/* ethernet */
    dp->hlen = 6;
    dp->xid = 0x4f496898;
    dp->secs = htons(5);
    dp->cookie = htonl(DHCP_MAGIC);
    memcpy(&(dp->chaddr[0]), &g_mac_address[0], 6 );
    
    for(i=0;i<(sizeof(dhcp_optflags)/sizeof(struct dhcp_opt_flag));i++) {
        up[offset] = dhcp_optflags[i].opt;
        offset++;
        if(dhcp_optflags[i].length) {
            up[offset] = dhcp_optflags[i].length;
            offset++;
            fill_options(&up[offset], dhcp_optflags[i].opt, DHCPDISCOVER );
            offset += dhcp_optflags[i].length;
        }
    }

}

static int get_padding(void)
{
    int offset = 0;
    int i;
    for(i=0;i<(sizeof(dhcp_optflags)/sizeof(struct dhcp_opt_flag));i++) {
        offset++;
        if(dhcp_optflags[i].length) {
            offset++;
            offset += dhcp_optflags[i].length;
        }
    }
    

    return offset;
}

int prepare_dhcpc_raw_packet(struct sk_buff *skb, unsigned char type)
{
    unsigned char * buf = (unsigned char *) skb->data;
    struct ip_hdr_s *iph;
    struct ip_udp_dhcp_s *packet;
    unsigned char dest_mac[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    /*prepare options*/
    int pading = get_padding();
    //printk("pading:%d\n", pading);
    pading += 31;
    
    iph = ip_hdr(skb);
    packet = (struct ip_udp_dhcp_s *)iph;

    switch(type) {
        case DHCPDISCOVER:
            /*construct ip header*/
            packet->ip.ihl = 5;
            packet->ip.version = 4;
            packet->ip.tos = 0;
            packet->ip.tot_len = ntohs((sizeof(struct ip_udp_dhcp_s) - (DHCP_OPTIONS_BUFSIZE - pading)));
            packet->ip.id = 0;
            packet->ip.frag_off = 0;
            packet->ip.ttl = 128;
            packet->ip.protocol = IPPROTO_UDP;
            packet->ip.saddr = 0;
            packet->ip.daddr = htonl(0xffffffff);
            packet->ip.check = inet_cksum((unsigned short *)&packet->ip, sizeof(packet->ip));
            
            /*construct udp header*/
            packet->udp.source = htons(68);
            packet->udp.dest = htons(67);
            packet->udp.len = htons(sizeof(struct udp_dhcp_packet)-(DHCP_OPTIONS_BUFSIZE - pading) );
            packet->ip.tot_len = packet->udp.len;
            packet->udp.check  = inet_cksum((unsigned short *)packet, (sizeof(struct ip_udp_dhcp_s) - (DHCP_OPTIONS_BUFSIZE - pading)));
            packet->ip.tot_len = htons( (sizeof(struct ip_udp_dhcp_s) - (DHCP_OPTIONS_BUFSIZE - pading)));
            
            /*construct hdcp struct*/
            prepare_bootp(skb, BOOTREQUEST );
            
            /*construct L2 header*/
            dhcp_construct_l2_header(skb, &g_mac_address[0], &dest_mac[0] );
            skb->length = (sizeof(struct ip_udp_dhcp_s)+14) - (DHCP_OPTIONS_BUFSIZE-pading) ;
            break;

        default:
            
            //break;
            return -1;
    }

    wmb();
    
    return 0;
}
#if 0
void pcnet32_init_test_packet(struct sk_buff *skb, int datalen) 
{
	unsigned char * buf = (unsigned char *) skb->data;
	int i;
        unsigned char tmp1[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
        unsigned char tmp[10] = {0x08, 0x06, 0x00, 0x01, 0x08, 0x00, 0x06, 0x04, 0x00, 0x01};
        unsigned char tmp2[4] = {0x0a, 0xb1, 0x1a, 0xc0};


        memcpy(&buf[0], &tmp1[0], sizeof(tmp1));
        buf+=6;
        memcpy(&buf[0], &g_mac_address[0], sizeof(g_mac_address));
        buf+=6;
        memcpy(&buf[0], tmp,sizeof(tmp));
        buf+=10;
        memcpy(&buf[0], &g_mac_address[0], sizeof(g_mac_address) );
        buf+=6;
        memset(&buf[0], 0x0, 10);

        buf += 10;
        memcpy(&buf[0], tmp2, sizeof(tmp2));        
        wmb();
	return;
    
}
#endif
