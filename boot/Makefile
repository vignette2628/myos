boot:	boot.s setup.s build_boot.c build_setup.c
	as86 -0 -a -o boot.o boot.s
	ld86 -0 -s -o boot boot.o
	
	as86 -0 -a -o setup.o setup.s
	ld86 -0 -s -o setup setup.o

	gcc  -Wall -O2 -fomit-frame-pointer -o build_boot build_boot.c 
	gcc -Wall -O2 -fomit-frame-pointer -o build_setup build_setup.c

	./build_boot boot bootsec
	./build_setup setup setupsec
	rm -f Image
	cat bootsec setupsec >./boot_partition
all:
	boot

clean:
	rm -rf *.o
	rm -rf boot setup build_boot build_setup bootsec setupsec boot_partition build_image
