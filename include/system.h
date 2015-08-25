#ifndef __SYSTEM_H__

#define __SYSTEM_H__

#ifndef NULL
#define NULL 0
#endif

typedef unsigned short u16;
typedef unsigned char u8;
typedef unsigned int u32;
typedef short s16;
typedef char s8;
typedef int s32;
typedef u32 dma_addr_t;

 /*网络字节序和主机字节序转换*/
static inline unsigned int ntohl(unsigned int v)
{
    return  ( (( ( v ) & 0xff) << 24) |(( ( v >> 8 ) & 0xff) << 16) | ( ( ( v >> 16) & 0xff) << 8) | ( ( v >> 24) & 0xff) );
}

static inline unsigned short ntohs(unsigned short v)
{
    return ((((v) & 0xff)<<8) |(((v) >> 8) & 0xff));
}

#define htonl   ntohl
#define htons  ntohs

/*保证内存访问*/
#define wmb()	__asm__ __volatile__ ("": : :"memory")

#define X86_FEATURE_XMM2	(0*32+26)
#define alternative(oldinstr, newinstr, feature)			\
	asm volatile ("661:\n\t" oldinstr "\n662:\n" 			\
		      ".section .altinstructions,\"a\"\n"		\
		      "  .align 4\n"					\
		      "  .long 661b\n"            /* label */		\
		      "  .long 663f\n"		  /* new instruction */	\
		      "  .byte %c0\n"             /* feature bit */	\
		      "  .byte 662b-661b\n"       /* sourcelen */	\
		      "  .byte 664f-663f\n"       /* replacementlen */	\
		      ".previous\n"					\
		      ".section .altinstr_replacement,\"ax\"\n"		\
		      "663:\n\t" newinstr "\n664:\n"   /* replacement */\
		      ".previous" :: "i" (feature) : "memory")
#define rmb() alternative("lock; addl $0,0(%%esp)", "lfence", X86_FEATURE_XMM2)

/*0x80中断*/
#define int80() __asm__ __volatile__ ("int $0x80"::)

/*设置中断返回地址cs:start_task()*/
#define move_to_start_task() \
__asm__ ("movl %%esp,%%eax\n\t" \
	"pushl $0x10\n\t" \
	"pushl %%eax\n\t" \
	"pushfl\n\t" \
	"pushl $0x08\n\t" \
	"pushl $start_task\n\t" \
	"iret\n" \
	:::"ax")

#define sti() __asm__ ("sti"::)
#define cli() __asm__ ("cli"::)
#define nop() __asm__ ("nop"::)

#define iret() __asm__ ("iret"::)



#define _set_gate(gate_addr,type,dpl,addr) \
__asm__ ("movw %%dx,%%ax\n\t" \
	"movw %0,%%dx\n\t" \
	"movl %%eax,%1\n\t" \
	"movl %%edx,%2" \
	: \
	: "i" ((short) (0x8000+(dpl<<13)+(type<<8))), \
	"o" (*((char *) (gate_addr))), \
	"o" (*(4+(char *) (gate_addr))), \
	"d" ((char *) (addr)),"a" (0x00080000))

#define set_intr_gate(n,addr) \
	_set_gate(&idt[n],14,0,addr)

#define set_trap_gate(n,addr) \
	_set_gate(&idt[n],15,0,addr)

	
#define _set_tssldt_desc(n,addr,type) \
__asm__ ("movw $104,%1\n\t" \
	"movw %%ax,%2\n\t" \
	"rorl $16,%%eax\n\t" \
	"movb %%al,%3\n\t" \
	"movb $" type ",%4\n\t" \
	"movb $0x00,%5\n\t" \
	"movb %%ah,%6\n\t" \
	"rorl $16,%%eax" \
	::"a" (addr), "m" (*(n)), "m" (*(n+2)), "m" (*(n+4)), \
	 "m" (*(n+5)), "m" (*(n+6)), "m" (*(n+7)) \
	)
#define lidt(_idt)  \
    __asm__ __volatile__ ( \
        "lidtl  (%%ebx)" \
        :   \
        :"b"(_idt)  \
        )
        
#define set_tss_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),"0x89")
#define set_ldt_desc(n,addr) _set_tssldt_desc(((char *) (n)),((int)(addr)),"0x82")

#define clear_nt() __asm__("pushfl ; andl $0xffffbfff,(%esp) ; popfl")

static inline void flush_write_buffers(void)
{
	__asm__ __volatile__ ("lock; addl $0,0(%%esp)": : :"memory");
}

extern inline void set_esp(unsigned int val)
{
    __asm__ __volatile__("movl %0, %%esp"::"r" ((unsigned int) val));
}

extern inline unsigned long get_esp() 
{
    unsigned  int _v;
    __asm__ __volatile__("movl %%esp, %0":"=r" (_v):);
    return _v;
}

extern inline unsigned long get_eax() 
{
    unsigned int _v;
    __asm__ __volatile__("movl %%eax, %0":"=r" (_v):);
    return _v;
}

extern inline  void set_eax(unsigned int val) 
{
    __asm__ __volatile__("movl %0, %%eax"::"r" ((unsigned int)val));
}

#endif
