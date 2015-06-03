;����
;1. ��ȡ��ǰ�ڴ��С
;2. ��Ӳ�̼����ں˴��뵽�ڴ��ַ: 0x1000:0000
;3. ���� 8259
;4. ����gdt�����뵽32bits����ģʽ����ת���ں˵�kernel_start()����



entry start

start:
	;setup.s��boot.s������ת�����ģ����Դ�ʱCS�Ĵ�����ֵ�Զ���Ϊ0x9020
	;Ϊ���ܷ���setup.s���������ݣ�Ҳ��Ҫˢ��DS,ES�Ĵ���Ϊ0x9020
	mov ax, cs
	mov ds, ax
	mov es, ax

	
	;��鵱ǰ�ڴ��С��ʹ��bios �ж�int 0x15���÷�����ȱ����:���֧��64M�ڴ�
	;����������memort_size��ǩ��Ӧ���ڴ��ַ�С�
	mov ah, #0x88
	mov al, #0x0
	int 0x15
	mov [memory_size], ax	

	;���Ӳ��CHS����(Cylinders, Heads, Sectors Per Track) �Ѵ��Ӳ�̲����ĵ�ֵַ����hd_info��ǩ��Ӧ���ڴ��ַ�С�
	;ֻ֧��CHSģʽ��ֻ֧�ֵ�һ��Ӳ��

	;��һ��Ӳ�̲�����ŵ������ַ��¼���ж�����0x41��(0x41*4 = �����ַ:0x0000:0104)

	;Դ��ַds:si = 0x0000:0x104
	mov	ax,#0x0000
	mov	ds,ax
	lds	si,[4*0x41]

	;Ŀ�ĵ�ַes:di = 0x9020:[hd_info]
	mov di, #hd_info

	mov	cx,#0x10 ;���Ƴ���
	rep
	movsb

	;�ָ�ds Ϊ0x9020
	mov ax, cs
	mov ds, ax

	;��ӡload kernel....��Ϣ
	mov si, #msg
	call #print_msg


	;׼�������ں˴��룬�ں˴���Ӵ��̵ĵ�4��������ʼ���
load_kernel:
	;��ʼ�� current_track and current_sector 
	mov al, #0x0
	mov byte [current_track], al
	mov al, #0x4
	mov byte [current_sector], al
	
	;��ȡ kernel_sectors,���ֵ�Ĵ�С�����ڽ����ں�image��ʱ����Ե���
	mov al, byte [kernel_sectors]
	push ax
	
	;����Ŀ�ĵ�ַΪ0x1000:0000,��֤��ǰ�ں��ļ���С����С��64kBytes 	
	mov ax, #0x1000
	mov es, ax
	xor bx, bx
	xor dx, dx
	
	mov dl, #0x0
	mov dh, #0x0	

	;��ʽ��ʼ�����ں�
read_begin:

	;from sector x, track 0, drive 0, head 0, ����x ��ֵ����� [current_sector]��
	xor cx, cx
	mov cl, byte [current_sector]
	mov ch, byte [current_track]

	;ÿ�μ���һ������
	xor ax, ax
	mov al, #0x1; one sector
	mov ah, #0x2;service 2
	
	int 0x13
	jc dead_loop_before 

	;�������һ�������ɹ����ӡ.
	call print_dot

	;ˢ�²�����Ƿ����е��������������
	dec byte [kernel_sectors] 
	jz read_finish

	;ˢ�� current_sector������Ƿ���Ҫ������һ���ŵ�
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
	;ˢ�� current_track	
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
	;��ԭ�����kernel_secotrsֵ
	pop ax
	mov byte [kernel_sectors], al

;�����ں˴��� �� 0x1000:0000 �� 0x0000:0000
;��Ҫ��֤�ں˴���С��64KBytes
	mov ax, #0x0000 
	cld
	
do_move:
	mov es, ax
	mov ax, #0x1000
	mov ds, ax
	sub di, di
	sub si, si
	mov cx, #0x8000 ;32768��ÿ��2���ֽ�
	rep
	movsw

	;���»ָ� ds �Ĵ���Ϊ0x9020�Ա�������gdt��gdt_48����
	mov ax, cs
	mov ds, ax

	;׼���л���32bits����ģʽ
	;���ж�
	cli
	;����idt��
	lidt idt_48
	;����gdt��
	lgdt gdt_48

;�����ú�8259��������ģʽ���´���ֱ��ʹ��linux 0.11����
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

	
	;�򿪱���ģʽ
	mov	ax,#0x0001	! protected mode (PE) bit
	lmsw	ax		! This is it!

	;��ת��cs:0000��ַ�� ����gdt������,����ģʽ�µ�CS�Ĵ���Ϊ8��DS�Ĵ���Ϊ0x10
	jmpi	0,8		

;��ӡmsg	
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

;���
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

;gdt�����òο� linux source code: setup.s, 2��entry����һ����CS���ڶ�����DS
gdt:
	.word	0,0,0,0		! dummy

	.word	0x3FFF		! 64Mb (4000*4096=64Mb)��������boot.s����ʶ�������ڴ�Ϊ64M
	.word	0x0000		! base address=0
	.word	0x9A00		! code read/exec
	.word	0x00C0		! granularity=4096, 386

	.word	0x3FFF		! 64Mb (4000*4096=64Mb)
	.word	0x0000		! base address=0
	.word	0x9200		! data read/write
	.word	0x00C0		! granularity=4096, 386

;�жϱ�Ĭ��Ϊ��
idt_48:
	.word	0			! idt limit=0
	.word	0,0			! idt base=0L

gdt_48:
	.word	0x800		! gdt limit=2048, 256 GDT entries
	.word	512+gdt,0x9	! gdt base = 0X9xxxx
	
		
	.org 1008
	hd_info: ;��¼�˵�һ��Ӳ�̵�CHS��Ϣ�Ĵ�ŵ�ַ
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
		
	;�ں˴�������ܳ���64k ���������127��sectors
	kernel_sectors:
		.byte 0x7f



