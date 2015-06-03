;			 功能:
;		1. 拷贝 boot.s 从 0x07c0:0000 到 0x9000:0000 
;		2. 加载 setup.s 到内存 0x9020:0000


;boot.s代码必须存放在磁盘的第一个扇区,且大小不能超过512字节。
;bios在每次启动后都会加载第一扇区内容到0x07c0:0000内存地址处
entry _start
	_start:

	;当前cpu还处在16bits 实模式,只能访问0000:0000到ffff:ffff范围大小的地址 
	;把0x7c00:0000地址起 512 bytes 拷贝到 0x9000:0000，然后继续执行

	;源地址
	mov ax, #0x07c0
	mov ds, ax

	;目的地址
	mov ax, #0x9000
	mov es, ax

	;清si和di
	sub si, si
	sub di, di
	
	;拷贝256次，每次2个字节
	mov cx, #256  ;
	rep 
	movw	

	
	;拷贝完后可以跳到 0x9000:go地址继续执行下面的代码了
	jmpi go, 0x9000

	go:

	;执行跳转指令后cs段的内容自动刷新为0x9000.
	mov ax, cs
	
	;刷新ds,es,ss段寄存器为0x9000
	mov ds, ax
	mov es, ax
	mov ss, ax
	
	;临时设置sp堆栈指针为:0x9000:ff00以便后面的函数调用
	mov sp, #0xff00


	;用 bios提供的 int 13h 中断把存放在磁盘的第2和第3个扇区的setup.s 加载到内存起始地址 0x9020:0000
	;假定用floopy disk并且setup.s大小不超过1024字节
	; ah = 2 read, ah = 3 write
	; al: section
	; ch: 
load_setup:

	;目标地址
	mov ax, #0x9020
	mov es, ax
	xor bx, bx

	; sector 2, track 0
	mov cx, #0x0002
	
	;drive 0, head 0
	mov dx, #0x0000
	
	;service 2
	mov ah, #0x2
	
	;需要加载2个扇区
	mov al, #0x2
	int 0x13

	;加载成功就跳转到load_setup_ok
	jnc load_setup_ok

	;如果加载失败就需要重新跳转到load_setup再次加载
	mov dx, #0x0000
	mov ax, #0x0000
	int 0x13
	jmp load_setup 

	;加载成功后就可以跳转到0x9020:0000处执行setup.s代码了。	
	load_setup_ok:

	;执行setup.s前调用bios中断int 0x10打印一点boot.s完成的信息。
	;恢复 es 寄存器 to 0x9000,否则无法正确访问到msg数据了。
	mov ax, #0x9000
	mov es, ax

	
	mov ah, #0x03
	xor bh, bh
	int 0x10

	;打印数据长度为24字节
	mov cx, #24 
	
	; page 0, attribute 7
	mov bx, #0x0007

	mov bp, #msg
	
	; write string and move cursor
	mov ax, #0x1301
	int 0x10	

	; 跳转到 0x9020:0000执行setup.s代码
	jmpi 0x0, 0x9020

	;要打印的数据定义区
	msg:
		.byte 13,10
		.ascii "Loading system ..."
		.byte 13,10,13,10


	;填充剩余的空间一直到510字节偏移处
	.org 510

	;根据bios的标准最后两个字节内容必须是0xaa55
	boot_flag:
		.word 0xaa55

	

	
