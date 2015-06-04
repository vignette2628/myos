INCLUDE = ../include 
CC	    = gcc -march=i386
LD      = ld


CCFLG   = -O2 -c -nostdinc -fno-builtin -I$(INCLUDE) -I.


COBJS   = main.o start32.o mem.o
OBJS    = $(COBJS) 


$(OBJS): main.c start32.s mem.c
	@echo
	@echo Making $<
	@$(CC) $(CCFLG) main.c -o main.o
	@$(CC) $(CCFLG) mem.c -o mem.o
	@$(CC) -I$(INCLUDE) -traditional -c start32.s
	@cp *.o ../obj

all: $(OBJS) 


clean:
	rm -f $(COBJS)
	rm -f *.o
