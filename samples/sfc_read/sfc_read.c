/**********************************************************
 * Author        : ylli
 * Email         : lee.ylli@ingenic.com
 * Last modified : 2022-11-05 21:25
 * Filename      : sfc_read.c
 * Description   :
 * *******************************************************/

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

#include "sfc_read.h"

static void dump_buf(uint32_t addr, uint32_t len, uint8_t *buf)
{
	uint32_t i;
	for(i=addr+1 ;i < addr+len+1 ;i++) {
		if((i-1)%16 == 0)
			PRINT_LOG_INFO("%08x: ",i-1);
		PRINT_LOG_INFO("%02x ",buf[i-1]);
		if(i%16 == 0)
			PRINT_LOG_INFO("\n");
	}
}

/**
 *  usage  :
 *	sample : ./sfc_read 0/1/2
 *	cmd    : 0:第一次读取需要申请buf; 1:再次读取不需要申请buf; 2:释放buf
 *	addr   : 读取flash的偏移地址
 *  len    : 读取flash的长度，注意长度要4K对齐
 **/
int main(int argc , char *argv[])
{
	struct sfc_transfer trans_config;
	uint8_t *buf, ret, cmd;

	cmd = atoi(argv[1]);

	int sfc_fd = open(SFC_DEV_PATH, O_RDONLY );
	if (sfc_fd < 0) {
		PRINT_LOG_INFO("open /dev/ingenic_sfc failed \n");
		return -1;
	}

	int memfd = open(MEM_DEV_PATH, O_RDWR | O_SYNC);
	if (memfd < 0) {
        PRINT_LOG_INFO("open /dev/mem failed!\n");
        return -1;
    }

	switch(cmd) {
		case 0:
			trans_config.status = MALLOC_READ;
			break;
		case 1:
			trans_config.status = BUF_READ;
			break;
		case 2:
			trans_config.status = FREE_BUF;
			ret = ioctl(sfc_fd, SFC_DO_READ, &trans_config);
			if (ret < 0) {
				PRINT_LOG_INFO("sfc ioctl free buf error ,ret = %d\n",ret);
				goto err_exit;
			}
			goto err_exit;
		default:
			PRINT_LOG_INFO("sfc ioctl read cmd error ,cmd = %d\n",cmd);
			break;
	}
	trans_config.addr = SFC_READ_ADDR;
	trans_config.len = SFC_READ_LEN;

	ret = ioctl(sfc_fd, SFC_DO_READ, &trans_config);
	if (ret < 0) {
		PRINT_LOG_INFO("sfc ioctl read flash addr:0x%x size:0x%x error \n",trans_config.addr,trans_config.len);
		goto err_exit;
	}

	trans_config.p_vir_buf_addr = (uint32_t)mmap(NULL, trans_config.len, PROT_READ | PROT_WRITE, MAP_SHARED, memfd, trans_config.d_phy_buf_addr);
	PRINT_LOG_DBG("retlen = 0x%x \n"             \
				  "d_phy_buf_addr = 0x%08x \n"   \
				  "d_vir_buf_addr = 0x%08x \n"   \
				  "p_vir_buf_addr = 0x%08x \n"   \
				  ,trans_config.retlen,trans_config.d_phy_buf_addr,trans_config.d_vir_buf_addr,trans_config.p_vir_buf_addr);

	if (trans_config.p_vir_buf_addr == (uint32_t)MAP_FAILED) {
		fprintf(stderr, "errno %d: %s\n", errno, strerror(errno));
		PRINT_LOG_INFO("mmap failed!\n");
		goto err_exit;
	}

	buf = (uint8_t *)trans_config.p_vir_buf_addr;
	dump_buf(0x0,0x100,buf);

	if (munmap((void *)trans_config.p_vir_buf_addr, trans_config.len) < 0)
		PRINT_LOG_INFO("munmap buf failed!\n");

err_exit:
    close(memfd);
	close(sfc_fd);
	return 0;
}

