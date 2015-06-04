#ifndef __HD_H__

#define __HD_H__

/*0x905f0: setup.s中hd_info标签位置.*/
#define HD_INFO_ADDR 0x905f0
#define MAX_HD_NUMBER 1

/*控制寄存器*/
#define HD_DATA		0x1f0	/* _CTL when writing */
#define HD_ERROR	0x1f1	/* see err-bits */
#define HD_NSECTOR	0x1f2	/* nr of sectors to read/write */
#define HD_SECTOR	0x1f3	/* starting sector */
#define HD_LCYL		0x1f4	/* starting cylinder */
#define HD_HCYL		0x1f5	/* high byte of starting cyl */
#define HD_CURRENT	0x1f6	/* 101dhhhh , d=drive, hhhh=head */
#define HD_STATUS	0x1f7	/* see status-bits */

#define HD_CMD		0x3f6

#define WIN_READ		0x20
#define WIN_WRITE		0x30
#define WIN_VERIFY		0x40
#define WIN_FORMAT		0x50


/* Bits of HD_STATUS */
#define ERR_STAT	0x01
#define INDEX_STAT	0x02
#define ECC_STAT	0x04	/* Corrected error */
#define DRQ_STAT	0x08
#define SEEK_STAT	0x10
#define WRERR_STAT	0x20
#define READY_STAT	0x40
#define BUSY_STAT	0x80

typedef struct hd_info_s {
    unsigned short cyl;
    unsigned char head;
    unsigned char sect;
    unsigned short wpcom;
    unsigned char ctl;
    unsigned short lzone;
}hd_info_t;

struct partition {
    unsigned char boot_ind;		/* 0x80 - active (unused) */
    unsigned char head;		/* ? */
    unsigned char sector;		/* ? */
    unsigned char cyl;		/* ? */
    unsigned char sys_ind;		/* ? */
    unsigned char end_head;		/* ? */
    unsigned char end_sector;	/* ? */
    unsigned char end_cyl;		/* ? */
    unsigned int start_sect;	/* starting sector counting from 0 */
    unsigned int nr_sects;		/* nr of sectors in partition */
};

/*只支持1个硬盘*/
extern hd_info_t g_hd_info[MAX_HD_NUMBER];
void hd_init(void);
void hd_chs_setup(void);
extern void hd_interrupt(void);
void hd_read_test(void);
void read_blk_test(void);
#endif
