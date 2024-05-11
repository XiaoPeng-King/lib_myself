/**********************************************************
 * Author        : kaiwang
 * Email         : kai.wang@ingenic.com
 * Last modified : 2020-09-15 21:25
 * Filename      : spi.c
 * Description   : 
 * *******************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>


char *device = "/dev/spidev1.0";
static  uint32_t mode = SPI_MODE_2;
static  uint8_t bits = 8;
static  uint32_t speed = 10*1000;
static  uint16_t delay = 0;
int fd = 0;
/*
 * 初始化SPI
 */
int spi_init(void)
{
    int ret = 0;
    /*打开 SPI 设备*/
    fd = open(device, O_RDWR);
    if (fd < 0)
    {
        printf("can't open /dev/spidev1.0 ");
    }

    /*
     * spi mode 设置SPI 工作模式
     */
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1){
        printf("can't set spi mode");
        return -1;
    }

    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1){
        printf("can't get spi mode");
        return -1;
    }

    /*
     * bits per word  设置一个字节的位数
     */
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1){
        printf("can't set bits per word");
        return -1;
    }

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1){
        printf("can't get bits per word");
        return -1;
    }

    /*
     * max speed hz  设置SPI 最高工作频率
     */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1){
        printf("can't set max speed hz");
        return -1;
    }

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1){
        printf("can't get max speed hz");
        return -1;
    }

    printf("spi mode: 0x%x\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed / 1000);
    return 0;
}
/**
 * spi 单字节发送
 * */
int  spi_write_byte(int fd,unsigned char data)
{
    int ret = 0;
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)&data,
        .rx_buf = 0,//SPI Can't support synchronous transfer
        .len = 1,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
    {
        printf("fd:%d can't send spi message\n",fd);
        return -1;
    }
    return 0;
}
/**
 * spi 单字节读
 * */
int  spi_read_byte(int fd,unsigned char data_cmd,unsigned char data)
{
    int ret = 0;
    struct spi_ioc_transfer tr[2];
    memset(tr,0,2*sizeof(struct spi_ioc_transfer));
    tr[0].tx_buf = (unsigned long)&data_cmd;
    tr[0].len = 1;
    tr[0].bits_per_word = bits,

    tr[1].tx_buf = 0;
    tr[1].rx_buf = (unsigned long)&data;
    tr[1].len = 1;
    tr[1].bits_per_word = bits,

    ret = ioctl(fd, SPI_IOC_MESSAGE(2), tr);
    if (ret < 1)
    {
        printf("fd:%d can't send spi message\n",fd);
        return -1;
    }
    return 0;
}

/**
 * spi 多字节发送
 * */
int  spi_write_bytes(int fd,unsigned char *pdata_buff,unsigned int len)
{
    int ret = 0;
    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)pdata_buff,
        .len = len,//SPI Can't support synchronous transfer
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1)
    {
        printf("fd:%d can't send spi message\n",fd);
        return -1;
    }
    return 0;
}

static void print_usage(const char *prog)
{
	printf("Usage: %s [-Dsbd]\n", prog);
	puts("  -D --device   device to use (default /dev/spidev1.1)\n"
	     "  -s --speed    max speed (Hz)\n"
	     "  -d --delay    delay (usec)\n"
	     "  -b --bpw      bits per word \n"
	     "for example ./spi --device /dev/spidev1.0 --speed 200000 --delay 0 --bpw 8 \n");
         exit(1);
}

static void parse_opts(int argc, char *argv[])
{
	while (1) {
		static const struct option lopts[] = {
			{ "device",  1, 0, 'D' },
			{ "speed",   1, 0, 's' },
			{ "delay",   1, 0, 'd' },
			{ "bpw",     1, 0, 'b' },
	    	{ NULL, 0, 0, 0 },
		};
		int c;

		c = getopt_long(argc, argv, "D:s:d:b:lHOLC3NR", lopts, NULL);

		if (c == -1)
			break;

		switch (c) {
		case 'D':
			device = optarg;
			break;
		case 's':
			speed = atoi(optarg);
			break;
		case 'd':
			delay = atoi(optarg);
			break;
		case 'b':
			bits = atoi(optarg);
			break;
	    default:
			print_usage(argv[0]);
            break;
		}
	}
}


unsigned char write_buff_test[4] = {0x01,0x02,0x03,0x04};
/**
 * SPI功能使用，支持标准Linux接口
 * sample_spi.c目的是测试spi功能,如果使用请按照具体开发需要更改
 * */
int main(int argc, char *argv[])
{
    int ret = 0;
    if(argc < 4){
        print_usage(argv[0]);
        return -1;
    }
    parse_opts(argc,argv);
    printf("set parse is : device %s ,speed %d ,delay %d ,bpw %d\n",device,speed,delay,bits);
    ret = spi_init();
    if(ret < 0){
        printf("spi init error\n");
        return 0;
    }
    spi_write_bytes(fd,write_buff_test,4);


    close(fd);

    return ret;
}
