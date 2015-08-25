INCLUDE = ../include 
CC	= gcc -march=i386
LD	= ld
AS	= as

CCFLG   = -O2 -c -nostdinc -fno-builtin -fstrength-reduce -fomit-frame-pointer -finline-functions -I$(INCLUDE) -I.



%.o: %.c
	@echo Making $<
	@$(CC) $(CCFLG) $< -o $@
	@cp *.o ../obj

%.o:%.s
	@echo Making $<
	@$(AS) -o $*.o $<
	@cp *.o ../obj
	
all: sched.o traps.o asm.o printk.o vsprintf.o  rx.o dhcpc.o command.o

clean:
	rm -f *.o
	rm -f ../obj/sched.o ../obj/asm.o ../obj/traps.o ../vsprintf.o ../rx.o ../dhcpc.o ../command.o

