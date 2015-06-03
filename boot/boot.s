;			 ����:
;		1. ���� boot.s �� 0x07c0:0000 �� 0x9000:0000 
;		2. ���� setup.s ���ڴ� 0x9020:0000


;boot.s����������ڴ��̵ĵ�һ������,�Ҵ�С���ܳ���512�ֽڡ�
;bios��ÿ�������󶼻���ص�һ�������ݵ�0x07c0:0000�ڴ��ַ��
entry _start
	_start:

	;��ǰcpu������16bits ʵģʽ,ֻ�ܷ���0000:0000��ffff:ffff��Χ��С�ĵ�ַ 
	;��0x7c00:0000��ַ�� 512 bytes ������ 0x9000:0000��Ȼ�����ִ��

	;Դ��ַ
	mov ax, #0x07c0
	mov ds, ax

	;Ŀ�ĵ�ַ
	mov ax, #0x9000
	mov es, ax

	;��si��di
	sub si, si
	sub di, di
	
	;����256�Σ�ÿ��2���ֽ�
	mov cx, #256  ;
	rep 
	movw	

	
	;�������������� 0x9000:go��ַ����ִ������Ĵ�����
	jmpi go, 0x9000

	go:

	;ִ����תָ���cs�ε������Զ�ˢ��Ϊ0x9000.
	mov ax, cs
	
	;ˢ��ds,es,ss�μĴ���Ϊ0x9000
	mov ds, ax
	mov es, ax
	mov ss, ax
	
	;��ʱ����sp��ջָ��Ϊ:0x9000:ff00�Ա����ĺ�������
	mov sp, #0xff00


	;�� bios�ṩ�� int 13h �жϰѴ���ڴ��̵ĵ�2�͵�3��������setup.s ���ص��ڴ���ʼ��ַ 0x9020:0000
	;�ٶ���floopy disk����setup.s��С������1024�ֽ�
	; ah = 2 read, ah = 3 write
	; al: section
	; ch: 
load_setup:

	;Ŀ���ַ
	mov ax, #0x9020
	mov es, ax
	xor bx, bx

	; sector 2, track 0
	mov cx, #0x0002
	
	;drive 0, head 0
	mov dx, #0x0000
	
	;service 2
	mov ah, #0x2
	
	;��Ҫ����2������
	mov al, #0x2
	int 0x13

	;���سɹ�����ת��load_setup_ok
	jnc load_setup_ok

	;�������ʧ�ܾ���Ҫ������ת��load_setup�ٴμ���
	mov dx, #0x0000
	mov ax, #0x0000
	int 0x13
	jmp load_setup 

	;���سɹ���Ϳ�����ת��0x9020:0000��ִ��setup.s�����ˡ�	
	load_setup_ok:

	;ִ��setup.sǰ����bios�ж�int 0x10��ӡһ��boot.s��ɵ���Ϣ��
	;�ָ� es �Ĵ��� to 0x9000,�����޷���ȷ���ʵ�msg�����ˡ�
	mov ax, #0x9000
	mov es, ax

	
	mov ah, #0x03
	xor bh, bh
	int 0x10

	;��ӡ���ݳ���Ϊ24�ֽ�
	mov cx, #24 
	
	; page 0, attribute 7
	mov bx, #0x0007

	mov bp, #msg
	
	; write string and move cursor
	mov ax, #0x1301
	int 0x10	

	; ��ת�� 0x9020:0000ִ��setup.s����
	jmpi 0x0, 0x9020

	;Ҫ��ӡ�����ݶ�����
	msg:
		.byte 13,10
		.ascii "Loading system ..."
		.byte 13,10,13,10


	;���ʣ��Ŀռ�һֱ��510�ֽ�ƫ�ƴ�
	.org 510

	;����bios�ı�׼��������ֽ����ݱ�����0xaa55
	boot_flag:
		.word 0xaa55

	

	
