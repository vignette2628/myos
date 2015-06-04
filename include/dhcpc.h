#ifndef __DHCPC_H__

#define __DHCPC_H__
#include <ip.h>
#define DHCP_MAGIC              0x63825363
#define DHCP_OPTIONS_BUFSIZE    308
#define BOOTREQUEST             1
#define BOOTREPLY               2

#define DHCPDISCOVER            1 /* client -> server */
#define DHCPOFFER               2 /* client <- server */
#define DHCPREQUEST             3 /* client -> server */
#define DHCPDECLINE             4 /* client -> server */
#define DHCPACK                 5 /* client <- server */
#define DHCPNAK                 6 /* client <- server */
#define DHCPRELEASE             7 /* client -> server */
#define DHCPINFORM              8 /* client -> server */
#define DHCP_MINTYPE DHCPDISCOVER
#define DHCP_MAXTYPE DHCPINFORM


/*options*/
#define DHCP_OPTION_51 0x33
#define DHCP_OPTION_51_LENGTH 4

#define DHCP_OPTION_53 0x35
#define DHCP_OPTION_53_LENGTH 1

#define DHCP_OPTION_55 0x37
#define DHCP_OPTION_55_LENGTH 17

#define DHCP_OPTION_255 0xff


struct dhcp_packet {
	unsigned char op;      /* BOOTREQUEST or BOOTREPLY */
	unsigned char htype;   /* hardware address type. 1 = 10mb ethernet */
	unsigned char hlen;    /* hardware address length */
	unsigned char hops;    /* used by relay agents only */
	unsigned int xid;    /* unique id */
	unsigned short secs;   /* elapsed since client began acquisition/renewal */
	unsigned short flags;  /* only one flag so far: */
	unsigned int ciaddr; /* client IP (if client is in BOUND, RENEW or REBINDING state) */
	unsigned int yiaddr; /* 'your' (client) IP address */
	/* IP address of next server to use in bootstrap, returned in DHCPOFFER, DHCPACK by server */
	unsigned int siaddr_nip;
	unsigned int gateway_nip; /* relay agent IP address */
	unsigned char chaddr[16];   /* link-layer client hardware address (MAC) */
	unsigned char sname[64];    /* server host name (ASCIZ) */
	unsigned char file[128];    /* boot file name (ASCIZ) */
	unsigned int cookie;      /* fixed first four option bytes (99,130,83,99 dec) */
	unsigned char options[DHCP_OPTIONS_BUFSIZE ];
} __attribute__((packed));

struct ip_udp_dhcp_s{
    ip_hdr_t ip;
    udp_hdr_t udp;
    struct dhcp_packet data;
}__attribute__((packed));

struct udp_dhcp_packet{
    struct udp_hdr_s udp;
    struct dhcp_packet data;
}__attribute__((packed));

struct dhcp_opt_flag {
    	unsigned char opt;
	unsigned char length;
};

int prepare_dhcpc_raw_packet(struct sk_buff *skb, unsigned char type);
#endif
