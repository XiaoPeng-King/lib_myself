#include <sys/types.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <errno.h>

#ifdef PRINT_DEBUG
#define  PRINT_LOG_DBG(format,arg...)			\
	printf(format,## arg)
#else
#define  PRINT_LOG_DBG(format,arg...)
#endif
#define  PRINT_LOG_INFO(format,arg...)			\
	printf(format,## arg)

//#define INGENIC_T31
#ifdef INGENIC_T31
#define SFC_DEV_PATH "/dev/jz_sfc"
#else
#define SFC_DEV_PATH "/dev/ingenic_sfc"
#endif

#define SFC_IOTC_CHECK 'S'
#define SFC_READ_SR   _IO(SFC_IOTC_CHECK, 0)
#define SFC_READ_SR1  _IO(SFC_IOTC_CHECK, 1)
#define SFC_READ_SR2  _IO(SFC_IOTC_CHECK, 2)

#define SFC_WRITE_SR _IO(SFC_IOTC_CHECK, 3)
#define SFC_WRITE_SR1 _IO(SFC_IOTC_CHECK, 4)
#define SFC_WRITE_SR2 _IO(SFC_IOTC_CHECK, 5)

#define SFC_DO_READ _IO(SFC_IOTC_CHECK, 6)

unsigned long simple_strtoul(const char *cp, char **endp,
				unsigned int base)
{
	unsigned long result = 0;
	unsigned long value;

	if (*cp == '0') {
		cp++;
		if ((*cp == 'x') && isxdigit(cp[1])) {
			base = 16;
			cp++;
		}

		if (!base)
			base = 8;
	}

	if (!base)
		base = 10;

	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
	    ? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}

	if (endp)
		*endp = (char *)cp;

	return result;
}

/**
 *  usage :
 *	argv[1] : 需要写哪个寄存器
 *	argv[2] : 根据手册看需要写入的8bit的寄存器的16进制值
 *	sample  : ./sfc_ioctl_test 0 0x7c
 **/
int main(int argc , char *argv[])
{
	uint8_t tmp, cmd=0xff, ret;
	uint8_t sr[3];
	char *endp;

	if(argc > 1)
		cmd = atoi(argv[1]);
	if(argc > 2) {
		tmp = simple_strtoul(argv[2], &endp, 16);
		if (*argv[1] == 0 || *endp != 0)
			return -1;
	}

	int sfc_fd = open(SFC_DEV_PATH, O_RDONLY );
	if (sfc_fd < 0)
		PRINT_LOG_INFO("open sfc fd error \n");

	/*** read status register***/
#if 1
	ret = ioctl(sfc_fd, SFC_READ_SR,sr);
	if (ret < 0) {
		PRINT_LOG_INFO("ioctl read error \n");
		goto err_exit;
	}
	PRINT_LOG_INFO("sr1 = 0x%02x, sr2 = 0x%02x, sr3 = 0x%02x\n",sr[0],sr[1],sr[2]);
#endif
	/*** write status register***/
	switch(cmd) {
	case 0:
		PRINT_LOG_INFO("------sr \n");
		ret = ioctl(sfc_fd, SFC_WRITE_SR, &tmp);
		if (ret < 0) {
			PRINT_LOG_INFO("ioctl write error \n");
			goto err_exit;
		}
		break;

	case 1:
		PRINT_LOG_INFO("------sr1 \n");
		ret = ioctl(sfc_fd, SFC_WRITE_SR1, &tmp);
		if (ret < 0) {
			PRINT_LOG_INFO("ioctl write error \n");
			goto err_exit;
		}
		break;

	case 2:
		PRINT_LOG_INFO("------sr2 \n");
		ret = ioctl(sfc_fd, SFC_WRITE_SR2, &tmp);
		if (ret < 0) {
			PRINT_LOG_INFO("ioctl write error \n");
			goto err_exit;
		}
		break;
	default:
		PRINT_LOG_INFO("----no cmd \n");
		break;
	}

err_exit:
	close(sfc_fd);
	return 0;
}

