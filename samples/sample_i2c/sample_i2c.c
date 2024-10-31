/**********************************************************
 * Author        : kaiwang
 * Email         : kai.wang@ingenic.com
 * Last modified : 2020-09-17 19:55
 * Filename      : sample_i2c.c
 * Description   : 
 * *******************************************************/
#include <stdio.h>
#include <linux/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

//#define I2C_24C02_DEV_ADDR 0x50   //i2c设备地址

#define I2C_TP_GT911_DEV_ADDR 0x28   //i2c设备地址,0x28,29


const char default_i2c[] = "/dev/i2c-2";
char *path = 0;

static int i2c_fd;
struct i2c_rdwr_ioctl_data i2c_data;
/**
 * i2c初始化函数
 *
 * */
int i2c_init(void)
{
	int fd = 0, ret = 0;
	if ((fd = open(path, O_RDWR)) < 0)
	{
		printf("open %s failed:%s\n", path, strerror(errno));
		return -1;
    }
    if ((ret = ioctl(fd, I2C_TIMEOUT, 100)) < 0) //超时时间
    {
        printf("set i2c timeout failed:%s\n", strerror(errno));
        close(fd);
        return -1;
    }
    if ((ret = ioctl(fd, I2C_RETRIES, 5)) < 0) //重复次数
    {
        printf("set i2c retries failed:%s\n", strerror(errno));
        close(fd);
        return -1;
    }
    if ((ret = ioctl(fd, I2C_TENBIT, 0)) < 0) //地址模式 7
    {
        printf("set i2c Address mode failed:%s\n", strerror(errno));
        close(fd);
        return -1;
    }
    i2c_fd=fd;
	return 0;
}
/**
 * 
 * */
int i2c_exit(void)
{
    if (i2c_fd > 0)
		close(i2c_fd);
	return 0;
}
/**
 * i2c 单字节读取
 * */
int i2c_read_byte(int fd,unsigned char dev_addr,unsigned char reg_addr,unsigned char *buf)
{
	int ret = 0;
	int dev_node = i2c_fd;
    struct i2c_msg message[2];

    message[0].addr = dev_addr;//设备地址
    message[0].flags = 0;   //写标志
    message[0].buf = &reg_addr;//寄存器地址
    message[0].len = sizeof(reg_addr);

    message[1].addr = dev_addr;//设备地址
    message[1].flags = I2C_M_RD; //读标志
    message[1].buf = buf;
    message[1].len = sizeof(reg_addr);

    i2c_data.msgs = message;
    i2c_data.nmsgs = 2;

    ret = ioctl(dev_node, I2C_RDWR, (unsigned long)&i2c_data);
    if (ret < 0)
    {
		printf("%s:%d I2C: read error:%s\n", __func__, __LINE__, strerror(errno));
		return ret;
	}
	return 0;
}
/**
 * i2c 多字节读取
 * */
int i2c_read_bytes(int fd,unsigned char dev_addr,unsigned char reg_addr,unsigned char *buf,unsigned int len)
{
	int ret = 0;
    struct i2c_msg message[2];

    message[0].addr = dev_addr;//设备地址
    message[0].flags = 0;   //写标志
    message[0].buf = &reg_addr;//寄存器地址
    message[0].len = sizeof(reg_addr);

    message[1].addr = dev_addr;//设备地址
    message[1].flags = I2C_M_RD; //读标志
    message[1].buf = buf;
    message[1].len = sizeof(unsigned char)*len;

    i2c_data.msgs = message;
    i2c_data.nmsgs = 2;

    ret = ioctl(fd, I2C_RDWR, (unsigned long)&i2c_data);
    if (ret < 0)
    {
		printf("%s:%d I2C: read error:%s\n", __func__, __LINE__, strerror(errno));
		return ret;
	}
	return 0;
}

/**
 *这里的寄存器地址是双字节 16bit
 * 
 * 
 * */
int i2c_read_bytes_dou_reg(int fd,unsigned char dev_addr,int reg_addr,unsigned char *buf,unsigned int len)
{
	int ret = 0;
    struct i2c_msg message[2];

    //寄存器地址高低位交换
    int tmp1, tmp2;
    tmp1 = ((reg_addr & 0xff00)>>8); // 0x1234 & 0xff00 = 0x1200 >> 8 = 0x0012
    tmp2 = ((reg_addr & 0x00ff)<<8); // 0x1234 & 0x00ff = 0x0034 << 8 = 0x3400
    reg_addr = tmp2 | tmp1;

    printf("tmp1: 0x%x tmp2: 0x%x \n", tmp1, tmp2);
    printf("reg_addr: 0x%x \n", reg_addr);


    message[0].addr = dev_addr;//设备地址
    message[0].flags = 0;   //写标志
    message[0].buf = &reg_addr;//寄存器地址
    message[0].len = 2;//sizeof(reg_addr); //这里只取前16bit，两个字节

    message[1].addr = dev_addr;//设备地址
    message[1].flags = I2C_M_RD; //读标志
    message[1].buf = buf;
    message[1].len = sizeof(unsigned char)*len;

    i2c_data.msgs = message;
    i2c_data.nmsgs = 2;

    ret = ioctl(fd, I2C_RDWR, (unsigned long)&i2c_data);
    if (ret < 0)
    {
		printf("%s:%d I2C: read error:%s\n", __func__, __LINE__, strerror(errno));
		return ret;
	}
	return 0;
}


/**
 * i2c 单字节写
 * */
int i2c_write_byte(int fd,unsigned char dev_addr,unsigned char reg_addr,unsigned char value_byte)
{
	int ret = 0;
    unsigned char buf[2]={0};
    struct i2c_msg message;
    
    buf[0] = reg_addr;//寄存器地址
    buf[1] = value_byte;//写入的字节
   
    message.addr = dev_addr;//设备地址
    message.buf = buf;
    message.flags = 0;//写标志
    message.len = 2;

    i2c_data.msgs = &message;
    i2c_data.nmsgs = 1;

	ret = ioctl(fd, I2C_RDWR,&i2c_data);
	if (ret < 0)
	{
		printf("%s:%d write data error:%s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return 0;
}

/**
 * i2c 多字节写
 * */
int i2c_write_bytes(int fd,unsigned char dev_addr,unsigned char reg_addr,unsigned char *p_value_bytes,unsigned int len)
{
	int ret = 0,i = 0;
    unsigned char buf[10]={0};
    struct i2c_msg message;
    
    buf[0] = reg_addr;//寄存器地址
    for(i = 0;i < len;i++){
        buf[i+1] = p_value_bytes[i];
    }
    message.addr = dev_addr;//设备地址
    message.buf = buf;
    message.flags = 0;//写标志
    message.len = len+1;//data+reg_addr

    i2c_data.msgs = &message;
    i2c_data.nmsgs = 1;

	ret = ioctl(fd, I2C_RDWR,&i2c_data);
	if (ret < 0)
	{
		printf("%s:%d write data error:%s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return 0;
}

/**
 * @brief 
 * 
 * 地址是16位：
 * 写入是高字节在前，低字节在后
 * 例如：
 *      设备地址：0x12 
 *      寄存器地址：0x2345
 * 
 * 这里的实际写入顺序是：
 *      0x12 0x45 0x23 
 * 
 * 
 */
int i2c_write_bytes_dou_reg(int fd,unsigned char dev_addr,int reg_addr, unsigned char *p_value_bytes,unsigned int len)
{
	int ret = 0,i = 0;
    unsigned char buf[10]={0};
    unsigned char reg_addr1, reg_addr2;
    struct i2c_msg message;
    
    //寄存器高低顺序调换
    reg_addr1 = (reg_addr & 0xFF00)>>8;
    reg_addr2 = (reg_addr & 0x00FF);
    printf("\nreg_addr1: 0x%x, reg_addr2: 0x%x\n", reg_addr1, reg_addr2);

    buf[0] = reg_addr1;//寄存器地址1
    buf[1] = reg_addr2;//寄存器地址2

    for(i = 0;i < len;i++){
        buf[i+2] = p_value_bytes[i];
        printf("buf[%d]= 0x%x \n", i+2, buf[i+2]);
    }
    
    message.addr = dev_addr;//设备地址
    message.buf = buf;
    message.flags = 0;//写标志
    message.len = len+2;//data+reg_addr

    i2c_data.msgs = &message;
    i2c_data.nmsgs = 1;

	ret = ioctl(fd, I2C_RDWR, &i2c_data);
	if (ret < 0)
	{
		printf("%s:%d write data error:%s\n", __func__, __LINE__, strerror(errno));
		return -1;
	}
	return 0;
}



/**
 * I2C功能使用，支持标准Linux接口
 * sample_i2c.c目的是测试I2C功能,如果使用请按照具体开发需要更改
 * 这里以24c02读写为例测试I2C功能
 * */
int main(int argc, char *argv[])
{
	int ret = 0;
    int i = 0;
    unsigned char read_byte = 0;
    unsigned char write_data_arr[10] = {0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17};
    unsigned char read_data_arr[10] = {0};
    if(argc > 1){
        path = argv[1];
    }else{
        path = (char *)default_i2c;
    }

    ret = i2c_init();
    if(ret < 0){
        printf("i2c_init failed\n");
        return 0;
    }
    /**
     * 单字节测试
     * */
    i2c_write_byte(i2c_fd,I2C_TP_GT911_DEV_ADDR,0x20,0xee);
    sleep(1);
    i2c_read_byte(i2c_fd,I2C_TP_GT911_DEV_ADDR,0x20,&read_byte);
    if(read_byte != 0xee){
        printf("read byte errno\n");
    }else {
        printf("check read_byte right\n");
    }
    /**
     * 多字节测试
     * */
    i2c_write_bytes(i2c_fd,I2C_TP_GT911_DEV_ADDR, 0x0, write_data_arr, 8);//设备地址+寄存器地址+写入数据
    sleep(1);
    i2c_read_bytes(i2c_fd,I2C_TP_GT911_DEV_ADDR,0x0,read_data_arr,8);
    for(i = 0;i < 8 ;i++ ){
        printf("read [%d] byte is 0x%x\n",i,read_data_arr[i]);
        if(read_data_arr[i] != write_data_arr[i]){
            printf("check data errno\n");
        }
    }
    i2c_exit();
    return ret;
}
