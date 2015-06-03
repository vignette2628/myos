#include <stdio.h>
#include <string.h>
#include <stdlib.h>	
#include <sys/types.h>	
#include <sys/stat.h>
#include <linux/fs.h>
#include <unistd.h>
#include <fcntl.h>

#define MINIX_HEADER_LEN 32
#define BOOT_LENGTH 512

int main(int argc, char *argv[] )
{
	char buf[1024];
	int fd;
	int len;

	if( argc != 3 ) {
		printf("no input file\n");
		return 0;
	}
	sprintf(buf,"rm -f %s", argv[2]);
	system(buf);
	sprintf(buf,"touch %s", argv[2]);
	system(buf);

	fd = open(argv[1], O_RDONLY, 0 );
	if( fd < 0 ){
		printf("open file:%s failed\n", argv[1]);
		return 0;
	}
	if( read(fd, buf, MINIX_HEADER_LEN)!= MINIX_HEADER_LEN) {
		close(fd);
		printf("Invalid mini header len:%d\n", MINIX_HEADER_LEN);
		return 0;
	}
	memset(buf, 0x0, sizeof(buf));
	len = read(fd, buf, sizeof(buf));
	if( len != BOOT_LENGTH ) {
		printf("wrong boot length:%d, should be %d\n", len, BOOT_LENGTH);
		return 0;
	}	

	close(fd);
        fd = open(argv[2], O_WRONLY, O_CREAT | O_EXCL | O_SYNC );
        if( fd < 0 ){
                printf("open file:%s failed\n", argv[2]);
                return 0;
        }

	len = write(fd, buf, BOOT_LENGTH);
	if( len != BOOT_LENGTH ){
		printf("write failed with len:%d, should be:%d\n", len, BOOT_LENGTH);
		return 0;
	}


	close(fd);
	return 0;
}
