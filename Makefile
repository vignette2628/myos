INCLUDE=./include

CC = gcc -march=i386
LD = ld

ASFLAG = -f elf
LDFLAGS = -melf_i386 -Ttext 0x0 -e startup_32
CCFLG = -O2 -c -nostdinc -fno-builtin -I$(INCLUDE) -I.


OBJ_DIR=./obj/
BOOT_DIR=./boot/
TOOLS_DIR = ./tools/
SCHED_DIR = ./schedule/
DIRIVER_DIR = ./drivers/

OBJS = $(OBJ_DIR)main.o $(OBJ_DIR)mem.o

LIBS = $(OBJ_DIR)sched.o $(OBJ_DIR)asm.o $(OBJ_DIR)traps.o $(OBJ_DIR)printk.o $(OBJ_DIR)vsprintf.o  $(OBJ_DIR)rx.o $(OBJ_DIR)dhcpc.o $(OBJ_DIR)command.o

DRIVERS = $(DRIVER_DIR)pcnet32.o $(DRIVER_DIR)keyboard.o $(DRIVER_DIR)hd.o $(DRIVER_DIR)console.o $(DRIVER_DIR)tty.o

all: Image

Image: boot_partition kernel $(TOOLS_DIR)build_image
	@echo
	@echo create Image
	@cat $(BOOT_DIR)boot_partition  kernel > ./Image
	@echo build image...
	@$(TOOLS_DIR)build_image Image
	@rm -rf Image.img
	@cp Image Image.img

boot_partition:
	@echo
	@echo Making boot...
	(cd boot; make clean; make;)

kernel: $(OBJS) $(LIBS) $(DRIVERS)
	@echo
	@echo Making kernel...
	@$(LD) $(LDFLAGS) $(OBJ_DIR)start32.o  $(OBJS) $(LIBS) $(OBJ_DIR)pcnet32.o $(OBJ_DIR)keyboard.o $(OBJ_DIR)hd.o $(OBJ_DIR)console.o $(OBJ_DIR)tty.o -o  $(OBJ_DIR)$@.pre
	@echo Dumping binnary ...
	@objdump -sD $(OBJ_DIR)$@.pre > $@.objdmp
	@echo Making binnary executable ...
	@objcopy -O binary -R .note -R .comment -S $(OBJ_DIR)$@.pre $@	

$(OBJS):
	(cd init;make clean;make)

$(LIBS):
	(cd schedule;make clean;make;)

$(DRIVERS):
	(cd drivers; make clean; make;)
	
$(TOOLS_DIR)build_image:
	(cd tools; make clean; make; )

clean:
	@echo
	@echo clean
	@rm -rf ./obj/*.o ./obj/*.pre
	@rm -f Image
	@rm -f *.objdmp
	@rm -f kernel
	@rm -f Image.img
	@cd drivers;make clean
	@cd schedule;make clean
	@cd init;make clean
	@cd boot;make clean

	
