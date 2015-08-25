#ifndef __START32_H__

#define __START32_H__
/*copy from linux source code: head.h*/

typedef struct desc_struct {
	unsigned long a,b;
} desc_table[256];
extern desc_table idt,gdt;

#define GDT_NUL 0
#define GDT_CODE 1
#define GDT_DATA 2
#define GDT_TMP 3

#define LDT_NUL 0
#define LDT_CODE 1
#define LDT_DATA 2

#endif
