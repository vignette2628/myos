#ifndef __IP_H__

#define __IP_H__
#include <mem.h>

#define MAX_INET_PROTOS	256
enum {
  IPPROTO_IP = 0,		/* Dummy protocol for TCP		*/
  IPPROTO_ICMP = 1,		/* Internet Control Message Protocol	*/
  IPPROTO_IGMP = 2,		/* Internet Group Management Protocol	*/
  IPPROTO_IPIP = 4,		/* IPIP tunnels (older KA9Q tunnels use 94) */
  IPPROTO_TCP = 6,		/* Transmission Control Protocol	*/
  IPPROTO_EGP = 8,		/* Exterior Gateway Protocol		*/
  IPPROTO_PUP = 12,		/* PUP protocol				*/
  IPPROTO_UDP = 17,		/* User Datagram Protocol		*/
  IPPROTO_IDP = 22,		/* XNS IDP protocol			*/
  IPPROTO_DCCP = 33,		/* Datagram Congestion Control Protocol */
  IPPROTO_RSVP = 46,		/* RSVP protocol			*/
  IPPROTO_GRE = 47,		/* Cisco GRE tunnels (rfc 1701,1702)	*/

  IPPROTO_IPV6	 = 41,		/* IPv6-in-IPv4 tunnelling		*/

  IPPROTO_ESP = 50,            /* Encapsulation Security Payload protocol */
  IPPROTO_AH = 51,             /* Authentication Header protocol       */
  IPPROTO_BEETPH = 94,	       /* IP option pseudo header for BEET */
  IPPROTO_PIM    = 103,		/* Protocol Independent Multicast	*/

  IPPROTO_COMP   = 108,                /* Compression Header protocol */
  IPPROTO_SCTP   = 132,		/* Stream Control Transport Protocol	*/
  IPPROTO_UDPLITE = 136,	/* UDP-Lite (RFC 3828)			*/

  IPPROTO_RAW	 = 255,		/* Raw IP packets			*/
  IPPROTO_MAX
};

typedef struct ip_hdr_s {
	unsigned char	ihl:4,
		version:4;
	unsigned char	tos;
	short	tot_len;
	short	id;
	short	frag_off;
	unsigned char	ttl;
	unsigned char	protocol;
	unsigned short check;
	int	saddr;
	int	daddr;
	/*The options start here. */
}ip_hdr_t;

typedef struct udp_hdr_s {
	short	source;
	short	dest;
	short	len;
	unsigned short 	check;
}udp_hdr_t;


#define ICMP_ECHOREPLY		0	/* Echo Reply			*/
#define ICMP_DEST_UNREACH	3	/* Destination Unreachable	*/
#define ICMP_SOURCE_QUENCH	4	/* Source Quench		*/
#define ICMP_REDIRECT		5	/* Redirect (change route)	*/
#define ICMP_ECHO		8	/* Echo Request			*/
#define ICMP_TIME_EXCEEDED	11	/* Time Exceeded		*/
#define ICMP_PARAMETERPROB	12	/* Parameter Problem		*/
#define ICMP_TIMESTAMP		13	/* Timestamp Request		*/
#define ICMP_TIMESTAMPREPLY	14	/* Timestamp Reply		*/
#define ICMP_INFO_REQUEST	15	/* Information Request		*/
#define ICMP_INFO_REPLY		16	/* Information Reply		*/
#define ICMP_ADDRESS		17	/* Address Mask Request		*/
#define ICMP_ADDRESSREPLY	18	/* Address Mask Reply		*/
#define NR_ICMP_TYPES		18

struct icmp_hdr_s {
  unsigned char		type;
  unsigned char		code;
  unsigned short	checksum;
  union {
	struct {
		short	id;
		short	sequence;
	} echo;
	int	gateway;
	struct {
		short	__unused;
		short	mtu;
	} frag;
  } un;
}icmp_hdr_t;

struct icmp_control_s {
	void (*handler)(struct sk_buff *skb);
	short   error;		/* This ICMP is classed as an error message */
};

struct net_protocol_s {
    int (*handler)(struct sk_buff *skb);
}net_protocol_t;

static inline ip_hdr_t *ip_hdr(const struct sk_buff *skb)
{
    return (ip_hdr_t *)((unsigned char *)(skb->data+14) );
}

static inline struct icmp_hdr_s *icmp_hdr(const struct sk_buff *skb)
{
	return (struct icmp_hdr_s *)(skb->data + 34);
}


#define NIPQUAD(addr) \
	((unsigned char *)&addr)[3], \
	((unsigned char *)&addr)[2], \
	((unsigned char *)&addr)[1], \
	((unsigned char *)&addr)[0]

#endif
