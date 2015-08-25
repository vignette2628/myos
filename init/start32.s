	/*����
	1. ��������gdt��idt����Ҫԭ����setup.s�е�gdt��idt���ݴ����0x090200��������ȫ�����ױ�OS��д��
	2. ��ת��kernel_start() [ main.c ]
	*/
.text
.globl idt, gdt

.globl startup_32
startup_32:
	/* ����setup.s �е�gdt���ã����ݶ�ds,es,fs,gs�ȶ�Ҫ���³�ʼ����0x10, CS�������ã�
	ԭ����startup_32����ת�����ģ�CS���Զ�ˢ�µ�0x8 */
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	lss stack_start, %esp
	call setup_idt
	call setup_gdt

	/*ģ��ret ���� kernel_start()ִ��,kernel_start()���غ����dead_loop()����ѭ����*/
	push $dead_loop
	push $kernel_start
	ret
	
dead_loop:
	jmp dead_loop


/*����idt��*/
setup_idt:
	lea ignore_int,%edx
	movl $0x00080000,%eax
	movw %dx,%ax		/* selector = 0x0008 = cs */
	movw $0x8E00,%dx	/* interrupt gate - dpl=0, present */

	lea idt,%edi
	mov $256,%ecx
rp_sidt:
	movl %eax,(%edi)
	movl %edx,4(%edi)
	addl $8,%edi
	dec %ecx
	jne rp_sidt
	lidt idt_descr
	ret

/*����gdt��*/
setup_gdt:

	lgdt gdt_descr
	ret

.align 2
ignore_int:
/*do nothing*/	
	iret
	
.align 2
.word 0


idt_descr:
	.word 256*8-1		# idt contains 256 entries
	.long idt
.align 2
.word 0
gdt_descr:
	.word 256*8-1		# so does gdt (not that that's any
	.long gdt		# magic number, but it works for me :^)

	.align 8
idt:	.fill 256,8,0x0		# idt is uninitialized

gdt:	.quad 0x0000000000000000	/* NULL descriptor */
	.quad 0x00c09a0000003fff	/* 64Mb */
	.quad 0x00c0920000003fff	/* 64Mb */
	.quad 0x0000000000000000	/* TEMPORARY - don't use */
	.fill 252,8,0			/* space for LDT's and TSS's etc */
