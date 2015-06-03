;功能
;1. 获取当前内存大小
;2. 从硬盘加载内核代码到内存地址: 0x1000:0000
;3. 配置 8259
;4. 配置gdt表并进入到32bits保护模式并跳转到内核的kernel_start()函数



entry start

start:
	;setup.s由boot.s代码跳转过来的，所以此时CS寄存器的值自动变为0x9020
	;为了能访问setup.s的数据内容，也需要刷新DS,ES寄存器为0x9020
	mov ax, cs
	mov ds, ax
	mov es, ax

	
	;检查当前内存大小，使用bios 中断int 0x15。该方法的缺点是:最大支持64M内存
	;检查结果存放在memort_size标签对应的内存地址中。
	mov ah, #0x88
	mov al, #0x0
	int 0x15
	mov [memory_size], ax	

	;检查硬盘CHS参数(Cylinders, Heads, Sectors Per Track) 把存放硬盘参数的地址值存在hd_info标签对应的内存地址中。
	;只支持CHS模式，只支持第一个硬盘

	;第一个硬盘参数存放的物理地址记录在中断向量0x41中(0x41*4 = 物理地址:0x0000:0104)

	;源地址ds:si = 0x0000:0x104
	mov	ax,#0x0000
	mov	ds,ax
	lds	si,[4*0x41]

	;目的地址es:di = 0x9020:[hd_info]
	mov di, #hd_info

	mov	cx,#0x10 ;复制长度
	rep
	movsb

	;恢复ds 为0x9020
	mov ax, cs
	mov ds, ax

	;打印load kernel....信息
	mov si, #msg
	call #print_msg


	;准备加载内核代码，内核代码从磁盘的第4个扇区开始存放
load_kernel:
	;初始化 current_track and current_sector 
	mov al, #0x0
	mov byte [current_track], al
	mov al, #0x4
	mov byte [current_sector], al
	
	;读取 kernel_sectors,这个值的大小可以在建立内核image的时候可以调整
	mov al, byte [kernel_sectors]
	push ax
	
	;设置目的地址为0x1000:0000,保证当前内核文件大小必须小于64kBytes 	
	mov ax, #0x1000
	mov es, ax
	xor bx, bx
	xor dx, dx
	
	mov dl, #0x0
	mov dh, #0x0	

	;正式开始加载内核
read_begin:

	;from sector x, track 0, drive 0, head 0, 其中x 的值存放在 [current_sector]中
	xor cx, cx
	mov cl, byte [current_sector]
	mov ch, byte [current_track]

	;每次加载一个扇区
	xor ax, ax
	mov al, #0x1; one sector
	mov ah, #0x2;service 2
	
	int 0x13
	jc dead_loop_before 

	;如果加载一个扇区成功则打印.
	call print_dot

	;刷新并检查是否所有的扇区都加载完毕
	dec byte [kernel_sectors] 
	jz read_finish

	;刷新 current_sector并检查是否需要跳到下一个磁道
	cmp cl, #18
	je next_track
	inc byte [current_sector]	
	
same_track:	
	;refresh bx register for offset
	add bx, #512

	;if bx = 0x0, 
	cmp bx, #0x0
	je next_64k
	jmp read_begin

next_track: 
	;刷新 current_track	
	push ax
	mov al, #0x1
	mov byte [current_sector], al
	pop ax
	inc dh
	and dh, #0x01
	cmp dh, #0x0
	jnz same_track
	
	inc byte [current_track]	
	
	jmp same_track

next_64k:
	push ax
	mov ax, es
	add ax, #0x1000
	mov es, ax
	pop ax
	jmp read_begin
	
dead_loop_before:
	;print err message
	
dead_loop:
	jmp dead_loop


read_finish:
	;还原最初的kernel_secotrs值
	pop ax
	mov byte [kernel_sectors], al

;拷贝内核代码 从 0x1000:0000 到 0x0000:0000
;需要保证内核代码小于64KBytes
	mov ax, #0x0000 
	cld
	
do_move:
	mov es, ax
	mov ax, #0x1000
	mov ds, ax
	sub di, di
	sub si, si
	mov cx, #0x8000 ;32768，每次2个字节
	rep
	movsw

	;重新恢复 ds 寄存器为0x9020以便后面访问gdt和gdt_48数据
	mov ax, cs
	mov ds, ax

	;准备切换到32bits保护模式
	;关中断
	cli
	;加载idt表
	lidt idt_48
	;加载gdt表
	lgdt gdt_48

;在配置好8259后开启保护模式以下代码直接使用linux 0.11代码
setup_8295:
	mov	al,#0x11		! initialization sequence
	out	#0x20,al		! send it to 8259A-1
	.word	0x00eb,0x00eb		! jmp $+2, jmp $+2
	out	#0xA0,al		! and to 8259A-2
	.word	0x00eb,0x00eb
	mov	al,#0x20		! start of hardware int's (0x20)
	out	#0x21,al
	.word	0x00eb,0x00eb
	mov	al,#0x28		! start of hardware int's 2 (0x28)
	out	#0xA1,al
	.word	0x00eb,0x00eb
	mov	al,#0x04		! 8259-1 is master
	out	#0x21,al
	.word	0x00eb,0x00eb
	mov	al,#0x02		! 8259-2 is slave
	out	#0xA1,al
	.word	0x00eb,0x00eb
	mov	al,#0x01		! 8086 mode for both
	out	#0x21,al
	.word	0x00eb,0x00eb
	out	#0xA1,al
	.word	0x00eb,0x00eb
	mov	al,#0xFF		! mask off all interrupts for now
	out	#0x21,al
	.word	0x00eb,0x00eb
	out	#0xA1,al

	
	;打开保护模式
	mov	ax,#0x0001	! protected mode (PE) bit
	lmsw	ax		! This is it!

	;跳转到cs:0000地址， 根据gdt表配置,保护模式下的CS寄存器为8，DS寄存器为0x10
	jmpi	0,8		

;打印msg	
print_msg:
	push ax
	mov ah, #0x0e
disp_next_ch:
	lodsb
	cmp al, #0
	jz  quit
	int 0x10
	jmp #disp_next_ch
quit:
	pop ax
	ret		

;打点
print_dot:    
    push ax    
    mov ax, #0x0e2e    
    int 0x10    
    pop ax    
    ret    


	msg:
		.byte 13,10
		.ascii "Loading kernel ..."
		.byte 0

	msg_ok:
		.ascii "ok"
		.byte 0

;gdt表配置参考 linux source code: setup.s, 2个entry，第一个给CS，第二个给DS
gdt:
	.word	0,0,0,0		! dummy

	.word	0x3FFF		! 64Mb (4000*4096=64Mb)，由于在boot.s中能识别的最大内存为64M
	.word	0x0000		! base address=0
	.word	0x9A00		! code read/exec
	.word	0x00C0		! granularity=4096, 386

	.word	0x3FFF		! 64Mb (4000*4096=64Mb)
	.word	0x0000		! base address=0
	.word	0x9200		! data read/write
	.word	0x00C0		! granularity=4096, 386

;中断表默认为空
idt_48:
	.word	0			! idt limit=0
	.word	0,0			! idt base=0L

gdt_48:
	.word	0x800		! gdt limit=2048, 256 GDT entries
	.word	512+gdt,0x9	! gdt base = 0X9xxxx
	
		
	.org 1008
	hd_info: ;记录了第一个硬盘的CHS信息的存放地址
		.word 0x1111
		.word 0x2222
		.word 0x3333
		.word 0x4444
		.word 0x5555
		
	memory_size:
		.word 0x1122
	
	current_track: 
		.byte 0x0
		
	current_sector:
		.byte 0x0
		
	retry_count:
		.byte 0x03
		
	;内核代码最大不能超过64k ，所以最大127个sectors
	kernel_sectors:
		.byte 0x7f



