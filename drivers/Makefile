INCLUDE = ../include 
CC	= gcc -march=i386
LD	= ld
AS	= as

CCFLG   = -O  -nostdinc -fno-builtin -fstrength-reduce -fomit-frame-pointer  -finline-functions -c -I$(INCLUDE) -I.



%.o: %.c
	@echo Making $<
	@$(CC) $(CCFLG) $< -o $@
	@cp *.o ../obj

%.o:%.s
	@echo Making $<
	@$(AS) -o $*.o $<
	@cp *.o ../obj
	
all: pcnet32.o keyboard.o hd.o console.o tty.o


clean:
	rm -f *.o
	rm -f ../obj/pcnet32.o
	rm -f ../obj/keyboard.o
	rm -f ../obj/hd.o
	rm -f ../obj/console.o
	rm -f ../obj/tty.o
