#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define FLOPPY_SIZE   1474560 /* 1.44*1024*1000 */


/*To create 1.44Mbytes files used to fit floppy*/
int main(int argc, char *argv[] )
{
    int fp;
    char *pbuf = NULL;
    int endpos = 0;
    if (argc != 2) {
        printf("use: create_image filename\n");
        return 0;
    }
    fp = open(argv[1], O_RDWR, O_CREAT | O_EXCL | O_SYNC);
    if (fp == -1) {
        perror("failed!\n");
        return 0;
    }

    endpos = lseek(fp, 0, SEEK_END);
    if( FLOPPY_SIZE < endpos ) {
        printf("Error: Image file too large with size:%d\n", endpos);
        return -1;
    }
    pbuf = malloc(FLOPPY_SIZE - endpos);
    memset(pbuf, 0, FLOPPY_SIZE - endpos);
    write(fp, pbuf, FLOPPY_SIZE - endpos);
    free(pbuf);
    close(fp);        
    return 0;
}
