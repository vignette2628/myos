#ifndef __ARP_H__

#define __ARP_H__

#define ETH_P_ARP	0x0806
#define ETH_P_IP	0x0800

#define	ARPOP_REQUEST	1		/* ARP request			*/
#define	ARPOP_REPLY	2		/* ARP reply			*/
#define ARP_TABLE_NUMBER 16

#define MAX_ARP_TIME_OUT 300        
typedef struct arp_table_s {
    unsigned int ip;
    unsigned int timeout;
    unsigned char mac[6];
}arp_table_t;

typedef struct arphdr_s
{
	unsigned short	 hardware_type;		
	unsigned short	 proto_type;		
	unsigned char	hardware_size;		
	unsigned char	proto_size;		
	unsigned short	 op_code;		


}arphdr_t;

#endif
