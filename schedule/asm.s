.globl default_handle, timer_interrupt, restore_esp, system_call_interrupt,pcnet32_interrupt,pcnet32_call_interrupt,keyboard_interrupt,keyboard_call_interrupt,hd_interrupt,hd_interrupt_handler

default_handle:
	call do_default_handle
	iret

.align 2
/*定时时间中断*/
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

	/*刷新jiffies计数器*/
	incl jiffies
	
	/*设置使8253能触发下一个中断*/
	movb $0x020, %al
	outb %al, $0x20

	/*当前esp存放到当前任务的current_esp中。current->current_esp*/
	mov %esp, %eax
	call save_esp

	/*调用do_timer()执行timer 少秒和任务调度并刷新current*/
	call do_timer

	/*将刷新后的current的current_esp恢复到esp*/	
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
/*0x80 软中断*/	
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
/*网卡中断*/
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
/*键盘中断*/	
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

/*硬盘中断处理*/
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
	