.globl default_handle, timer_interrupt, restore_esp, system_call_interrupt,pcnet32_interrupt,pcnet32_call_interrupt,keyboard_interrupt,keyboard_call_interrupt,hd_interrupt,hd_interrupt_handler

default_handle:
	call do_default_handle
	iret

.align 2
/*��ʱʱ���ж�*/
timer_interrupt:

	push %ds
	push %es
	push %fs
	push %gs

	pushl %ebp		
	pushl %edi		
	pushl %esi	
	
	pushl %edx		
	pushl %ecx		
	pushl %ebx		
	pushl %eax

	/*ˢ��jiffies������*/
	incl jiffies
	
	/*����ʹ8253�ܴ�����һ���ж�*/
	movb $0x020, %al
	outb %al, $0x20

	/*��ǰesp��ŵ���ǰ�����current_esp�С�current->current_esp*/
	mov %esp, %eax
	call save_esp

	/*����do_timer()ִ��timer �����������Ȳ�ˢ��current*/
	call do_timer

	/*��ˢ�º��current��current_esp�ָ���esp*/	
	call restore_esp
	mov %eax, %esp
	
	popl %eax
	popl %ebx
	popl %ecx
	popl %edx

	popl %edi
	popl %esi
	popl %ebp

	pop %gs
	pop %fs
	pop %es
	pop %ds
	
	iret
/*0x80 ���ж�*/	
system_call_interrupt:	

	push %ds
	push %es
	push %fs
	push %gs

	pushl %ebp		
	pushl %edi		
	pushl %esi	
	
	pushl %edx		
	pushl %ecx		
	pushl %ebx		
	pushl %eax	

	mov %esp, %eax
	call save_esp
	call do_syscall_timer
	call restore_esp
	mov %eax, %esp
	
	popl %eax
	popl %ebx
	popl %ecx
	popl %edx

	popl %edi
	popl %esi
	popl %ebp

	pop %gs
	pop %fs
	pop %es
	pop %ds
	
	iret
/*�����ж�*/
pcnet32_call_interrupt:

	push %ds
	push %es
	push %fs
	push %gs

	pushl %ebp		
	pushl %edi		
	pushl %esi	
	
	pushl %edx		
	pushl %ecx		
	pushl %ebx		
	pushl %eax	

	call pcnet32_interrupt
	
	popl %eax
	popl %ebx
	popl %ecx
	popl %edx

	popl %edi
	popl %esi
	popl %ebp

	pop %gs
	pop %fs
	pop %es
	pop %ds

	iret
/*�����ж�*/	
keyboard_call_interrupt:
	push %ds
	push %es
	push %fs
	push %gs

	pushl %ebp		
	pushl %edi		
	pushl %esi	
	
	pushl %edx		
	pushl %ecx		
	pushl %ebx		
	pushl %eax	

	call keyboard_interrupt

	popl %eax
	popl %ebx
	popl %ecx
	popl %edx

	popl %edi
	popl %esi
	popl %ebp

	pop %gs
	pop %fs
	pop %es
	pop %ds
	iret

/*Ӳ���жϴ���*/
hd_interrupt:
	push %ds
	push %es
	push %fs
	push %gs

	pushl %ebp		
	pushl %edi		
	pushl %esi	
	
	pushl %edx		
	pushl %ecx		
	pushl %ebx		
	pushl %eax	

	call hd_interrupt_handler
	popl %eax
	popl %ebx
	popl %ecx
	popl %edx

	popl %edi
	popl %esi
	popl %ebp

	pop %gs
	pop %fs
	pop %es
	pop %ds
	iret	
	