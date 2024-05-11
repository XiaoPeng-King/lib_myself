#ifndef __SFC_READ_H_
#define __SFC_READ_H_

#define SFC_IOTC_CHECK       'S'
#define SFC_READ_SR          _IO(SFC_IOTC_CHECK, 0)
#define SFC_READ_SR1         _IO(SFC_IOTC_CHECK, 1)
#define SFC_READ_SR2         _IO(SFC_IOTC_CHECK, 2)

#define SFC_WRITE_SR         _IO(SFC_IOTC_CHECK, 3)
#define SFC_WRITE_SR1		 _IO(SFC_IOTC_CHECK, 4)
#define SFC_WRITE_SR2        _IO(SFC_IOTC_CHECK, 5)

#define SFC_DO_READ          _IO(SFC_IOTC_CHECK, 6)

#define SFC_DEV_PATH         "/dev/ingenic_sfc"
#define MEM_DEV_PATH         "/dev/mem"

#define SFC_READ_ADDR        0x0
#define SFC_READ_LEN         0x1000000

#ifdef PRINT_DEBUG
#define  PRINT_LOG_DBG(format,arg...)			\
	printf(format,## arg)
#else
#define  PRINT_LOG_DBG(format,arg...)
#endif
#define  PRINT_LOG_INFO(format,arg...)			\
	printf(format,## arg)

struct sfc_transfer {

	uint32_t addr;
	uint32_t len;
	uint32_t retlen;
	uint8_t  status;
	uint32_t p_vir_buf_addr;
	uint32_t d_vir_buf_addr;
	uint32_t d_phy_buf_addr;
};

enum {
	MALLOC_READ,
	BUF_READ,
	FREE_BUF,
};

#endif /* __SFC_READ_H_ */

