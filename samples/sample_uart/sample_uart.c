/**********************************************************
 * Author        : kaiwang
 * Email         : kai.wang@ingenic.com
 * Last modified : 2020-09-17 20:03
 * Filename      : sample_uart.c
 * Description   : 
 * *******************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <termios.h>
#include <string.h>
#include <sys/ioctl.h>

const char default_path[] = "/dev/ttyS2";

int baud_rate[] = {B115200,B57600,B38400,B19200,B9600,B4800,B2400,B1200,B300};
int baud_rate_num[] = {115200,57600,38400,19200,9600,4800,2400,1200,300};

int fd;
char buf[1024] = "Ingenic tty send test.\n";
char *path;

#define CSTOPB 0000100
/* c_cflag bit meaning */
#define PARENB 0000400
#define PARODD 0001000
/* c_iflag bits */
#define INPCK 0000020

#define CSIZE 0000060
#define CS5 0000000
#define CS6 0000020
#define CS7 0000040
#define CS8 0000060
/**
* uart init
* */

int uart_init(int baudrate,int databits,int stopbits,char parity)
{
    int i = 0;
    /*
     * 获取串口设备描述符
     * Get serial device descriptor
     * */    
    printf("This is tty/usart demo.\n");
    fd = open(path, O_RDWR);
    if (fd < 0) {
        printf("Fail to Open %s device\n", path);
        return -1;
    }
    struct termios opt;
    /*
     * 清空串口接收缓冲区
     * Clear the serial port receive buffer
     * */
    tcflush(fd, TCIOFLUSH);
    tcgetattr(fd, &opt);
    /**
     * 设置串口波特率
     * Set the serial port baud rate
     * */
    for(i = 0; i < sizeof(baud_rate)/sizeof(int) ;i++){
        if(baud_rate_num[i] == baudrate){
            cfsetospeed(&opt, baud_rate[i]);
            cfsetispeed(&opt, baud_rate[i]);
        }
    }
    /*
     * 设置数据位数,先清除CSIZE数据位的内容
     * Set the number of data bits, first clear the content of the CSIZE data bit
     * */    
    opt.c_cflag &= ~CSIZE;
    switch (databits)
    {
    case 5:
        opt.c_cflag |= CS5;
        break;
    case 6:
        opt.c_cflag |= CS6;
        break;
    case 7:
        opt.c_cflag |= CS7;
        break;
    case 8:
        opt.c_cflag |= CS8;
        break;
    }
    /* 
     * 设置停止位
     * Set stop bit
     **/
    switch (stopbits)
    {
    case 1:
        opt.c_cflag &= ~CSTOPB;//1位 1 bit
        break;
    case 2:
        opt.c_cflag |= CSTOPB;//2位 2 bit
        break;
    }
    /*
     * 设置校验位
     * Set check digit
     * */
    switch (parity)
    {
    case 'n':
    case 'N':
        opt.c_cflag &= ~PARENB; // 不使用奇偶校验 
        opt.c_iflag &= ~INPCK;  // 禁止输入奇偶检测
        break;
    case 'o':
    case 'O':
        opt.c_cflag |= PARENB;  // 启用奇偶效验 
        opt.c_iflag |= INPCK;   // 启用输入奇偶检测 
        opt.c_cflag |= PARODD ; // 设置为奇效验 
        break;
    case 'e':
    case 'E':
        opt.c_cflag |= PARENB;  // 启用奇偶效验
        opt.c_iflag |= INPCK;   // 启用输入奇偶检测
        opt.c_cflag &= ~PARODD; // 设置为偶效验
        break;
    }

    /*
     *更新配置
     *Update configuration
     **/    
    tcsetattr(fd, TCSANOW, &opt);

    printf("Device %s is set to %d bps,databits %d,stopbits %d,parity %c\n",path,baudrate,databits,stopbits,parity);
    return 0;
}

/**
 * @brief 串口发送函数
 *
 *
 * */
int uart_send(int fd,char *send_buf,int data_len)
{
    int len = 0;
    len = write(fd,send_buf,data_len);
    if(len == data_len){
        return len;
    } else{
        tcflush(fd,TCIOFLUSH);
        return -1;
    }
    return 0;
}
/*
 * 串口接收函数
 *
 * **/

int uart_receive(int fd,char *rev_buff,int data_len)
{
    int bytes = 0;
    int ret = 0;
    ioctl(fd,FIONREAD,&bytes);//不阻塞
    if(bytes > 0){
        ret = read(fd,rev_buff,data_len);
    }
    return ret;
}
int main(int argc, char *argv[])
{
    int res;
    int set_baudrate = 115200;
    int set_databits = 8;
    int set_stopbits = 1;
    char set_parity = 'N';
    path = (char *)default_path;

    if (argc > 4){
        path = argv[1];
        set_baudrate = atoi(argv[2]);
        set_databits = atoi(argv[3]);
        set_stopbits = atoi(argv[4]);
        set_parity = *argv[5];
    } else {
        printf("For example: ./sample_uart /dev/ttyS2 115200 8 1 N\n"); 
        return 0;
    }
    /*
     * 串口初始化函数
     * uart init function
     * */
    res = uart_init(set_baudrate,set_databits,set_stopbits,set_parity);
    if(res < 0){
        printf("uart init failed \n");
    }

    /*
     * 测试代码
     * test code
     */
    do {
        //发送字符串
        printf("send buf: %s\n",buf);
        write(fd, buf, strlen(buf));
        //接收字符串
        res = read(fd, buf, 1024);
        if (res >0 ) {
            buf[res] = '\0';
            printf("Receive res = %d bytes data: %s\n",res, buf);
        }
        memcpy(buf,"Ingenic tty send test.\n",sizeof("Ingenic tty send test.\n"));       
    }while (res >= 0);

    printf("read error,res = %d",res);

    close(fd);
    return 0;
}
