/*
* Copyright (C) 2013 - 2016  Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person
* obtaining a copy of this software and associated documentation
* files (the "Software"), to deal in the Software without restriction,
* including without limitation the rights to use, copy, modify, merge,
* publish, distribute, sublicense, and/or sell copies of the Software,
* and to permit persons to whom the Software is furnished to do so,
* subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
* CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in this
* Software without prior written authorization from Xilinx.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/mman.h>
#include <errno.h>

#define MEM_TEST_VERSION  "V1.3"

static int dev_fd;
int main(int argc, char **argv)
{
    int flag = 0;
    int i;
    unsigned int *map_base, addr, value, map_size;;
    unsigned long base;

    printf("mem-test version: %s\n", MEM_TEST_VERSION);

	//printf("int/long/float/double/longlong: %d/%d/%d/%d/%d\n", sizeof(int), sizeof(long), sizeof(float), sizeof(double), sizeof(long long));
    if (argc != 5) {
        printf("usage:   mem-test(hex) r base(64bit) offset(32bit) read_num(32bit)\n");
        printf("example: mem-test r 0x50008000 20 1\n");
        printf("usage:   mem-test(hex) w base(64bit) offset(32bit) write_val(32bit)\n");
        printf("example: mem-test w 0x50008000 20 0xffffffff\n");
        return -1;
    }

    if (argv[1][0] == 'r') {
        flag = 0;
    }
    else {
        flag = 1;
    }

    base = strtol(argv[2], NULL, 16);
    addr = strtol(argv[3], NULL, 16);
    value = strtol(argv[4], NULL, 16);

    dev_fd = open("/dev/mem", O_RDWR | O_NDELAY | O_DSYNC);

    if (dev_fd < 0)
    {
        printf("open(/dev/mem) failed.");
        return 0;
    }

    addr &= ~0x3;
    map_size = addr + 0x100;
    printf("map base: 0x%lx, size: 0x%x\n", base, map_size);
    map_base = (unsigned int *)mmap(NULL, map_size, PROT_READ | PROT_WRITE, MAP_SHARED, dev_fd, base);
    if((long)map_base == -1)
    {
        printf("map err %d\n",errno);
        perror("errno:");
        return -1;
    }

    if (flag == 0) {

        for (i = 0; i < value; i++) {
            if (i % 4 == 0) {
                printf("\n");
                printf("0x%08x\t", addr+i*4);
            }
            printf("%08x ", *(volatile unsigned int *)(map_base+addr+i*4));
        }
        printf("\n");

    }
    else {
        printf("0x%08x\t value:0x%x", addr,value);
        *(volatile unsigned int *)(map_base + addr) = value;
    }
    printf("\n");

    if(dev_fd)
        close(dev_fd);

    munmap((unsigned int *)map_base, map_size);

    return 0;
}

