#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>


#if 1 //红绿蓝测试程序

struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;     

typedef struct lcd_color
{
    unsigned char bule;
    unsigned char green;
    unsigned char red;
    unsigned char alpha;
} lcd_color;
 
/**
 * 更新屏幕显示内存块信息，颜色格式为RGB8888
*/
void screen_refresh(char *fbp, lcd_color color_buff, long screen_size)
{
    for(int i=0; i < screen_size; i+=4)
    {
        *((lcd_color*)(fbp + i)) = color_buff;
    }
    usleep(1000*2000);
}

int init_framebuffer(void)
{
	int fp = open("/dev/fb0", O_RDWR);
 
    if (fp < 0)
    {
        printf("Error : Can not open framebuffer device/n");
        exit(1);
		return -1;
    }
 
    if (ioctl(fp, FBIOGET_FSCREENINFO, &finfo))
    {
        printf("Error reading fixed information/n");
        exit(2);
		return -1;
    }
 
    if (ioctl(fp, FBIOGET_VSCREENINFO, &vinfo))
    {
        printf("Error reading variable information/n");
        exit(3);
		return -1;
    }

	/* 打印获取的屏幕信息 */
	printf("The xres is :%d\n", vinfo.xres); //屏幕分辨率 横向
    printf("The yres is :%d\n", vinfo.yres); //屏幕分辨率 纵向
	printf("The mem is :%d\n", finfo.smem_len); //
    printf("The line_length is :%d\n", finfo.line_length);
	printf("bits_per_pixel is :%d\n", vinfo.bits_per_pixel);

	return fp;
}

int main(void)
{
	int ret = -1;
    int rgb_type = 0;
    long screen_size = 0; 
    
	int fp;
    unsigned char *fbp = 0;
 	
	fp = init_framebuffer();
	if (fp < 0)
	{
		return 0;
	}

    /* 获取RGB的颜色颜色格式，比如RGB8888、RGB656 */
    rgb_type = vinfo.bits_per_pixel / 8;

    /* 屏幕的像素点 */
    screen_size = vinfo.xres * vinfo.yres * rgb_type;

    /* 映射 framebuffer 的缓冲空间，得到一个指向这块空间的指针 */
    fbp =(unsigned char *) mmap (NULL, screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fp, 0);
    if (fbp == NULL)
    {
       printf ("Error: failed to map framebuffer device to memory./n");
       exit (4);
    }
 
    /* 刷白屏 */
    memset(fbp, 0xff, screen_size); //显存内部全部填充1时屏幕默认白屏
    usleep(1000*2000);
	memset(fbp, 0x00, screen_size); //显存内部全部填充0时屏幕默认黑屏

 #if 0
    /* 显示屏是RGDA的，所以显色格式为32为，注意自己的显示屏信息，对应修改 */
    /* 刷红色 */
    screen_refresh(fbp, (lcd_color){0, 0, 255, 255}, screen_size); //32bit
    /* 刷绿色 */
    screen_refresh(fbp, (lcd_color){0, 255, 0, 255}, screen_size);
 
    /* 刷蓝色 */
    screen_refresh(fbp, (lcd_color){255, 0, 0, 255}, screen_size);
#endif
#if 0
	//屏幕色深 16bit RGB565 11111 111111 11111 / 1111 1000 0000 0000 
	//刷红色
	screen_refresh(fbp, (lcd_color){248, 0}, screen_size); //16bit 1111 1000 0000 0000 = 
    /* 刷蓝 */
    screen_refresh(fbp, (lcd_color){7, 300}, screen_size); // 0000 0111 1100 0000
    /* 刷绿 */
    screen_refresh(fbp, (lcd_color){0, 37}, screen_size); //0000 0000 0001 1111
 #endif

#if 1
	//屏幕色深 16bit RGB555 0 11111 11111 11111 / 1111 1000 0000 0000 
	//刷红色
	printf("red\n");
	screen_refresh(fbp, (lcd_color){124, 0}, screen_size); //16bit 0111 1100 0000 0000 = 
    /* 刷蓝 */
	printf("blue\n");
    screen_refresh(fbp, (lcd_color){3, 224}, screen_size); // 0000 0011 1110 0000
    /* 刷绿 */
	printf("green\n");
    screen_refresh(fbp, (lcd_color){0, 37}, screen_size); //0000 0000 0001 1111
	printf("black\n");
	screen_refresh(fbp, (lcd_color){0, 0}, screen_size); //0000 0000 0000 0000
	printf("whiten");
	screen_refresh(fbp, (lcd_color){255, 255}, screen_size); //0000 0000 0001 1111


 #endif

    /* 解除映射 */
    munmap (fbp, screen_size); 
 
    close(fp);
    return 0;
}
 

#endif 



#if 0
static int fd_fb;
static struct fb_var_screeninfo var;	/* Current var */
static int screen_size;					/* 一帧数据所占用的字节数*/
static unsigned char *fb_base;			/* Framebuffer首地址*/
static unsigned int line_width;			/* 一行数据所占用的字节数*/
static unsigned int pixel_width;		/* 单个像素所占用的字节数*/

/**********************************************************************
 * 函数名称： lcd_put_pixel
 * 功能描述： 在LCD指定位置上输出指定颜色（描点）
 * 输入参数： x坐标，y坐标，颜色
 * 输出参数： 无
 * 返 回 值： 会
 * 修改日期        版本号     修改人	      修改内容
 * -----------------------------------------------
 * 2020/05/12	     V1.0	  zh(angenao)	      创建
 ***********************************************************************/ 
void lcd_put_pixel(int x, int y, unsigned int color){
	unsigned char *pen_8 = fb_base+y*line_width+x*pixel_width;
	unsigned short *pen_16;	
	unsigned int *pen_32;	

	unsigned int red, green, blue;	

	pen_16 = (unsigned short *)pen_8;
	pen_32 = (unsigned int *)pen_8;

	switch (var.bits_per_pixel){
		case 8:{
			*pen_8 = color;
			break;
		}
		case 16:{
			/* 565 */
			red   = (color >> 16) & 0xff;
			green = (color >> 8) & 0xff;
			blue  = (color >> 0) & 0xff;
			color = ((red >> 3) << 11) | ((green >> 2) << 5) | (blue >> 3);
			*pen_16 = color;
			break;
		}
		case 32:{
			*pen_32 = color;
			break;
		}
		default:{
			printf("can't surport %dbpp\n", var.bits_per_pixel);
			break;
		}
	}
}

int main(int argc, char **argv){
	int i;
	fd_fb = open("/dev/fb0", O_RDWR);/** 打开fb设备*/
	if (fd_fb < 0){
		printf("can't open /dev/fb0\n");
		return -1;
	}
	if (ioctl(fd_fb, FBIOGET_VSCREENINFO, &var)){/** 获取屏幕可变信息*/
		printf("can't get var\n");
		return -1;
	}
	printf("RES:%d x %d\n",var.xres,var.yres);
	printf("one pixel bits:%d\n",var.bits_per_pixel);
	line_width  = var.xres * var.bits_per_pixel / 8;// 一行数据 占据字节数
	printf("line_width:%d byte\n",line_width);
	pixel_width = var.bits_per_pixel / 8;///单个像素占用的字节数
	printf("pixel_width:%d byte\n",pixel_width);
	screen_size = var.xres * var.yres * var.bits_per_pixel / 8;//一帧画面占用的字节数
	printf("screen_size:%d byte\n",screen_size);
	fb_base = (unsigned char *)mmap(NULL , screen_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd_fb, 0);/** 映射framebuffer的首地址*/
	if (fb_base == (unsigned char *)-1){
		printf("can't mmap\n");
		return -1;
	}
	memset(fb_base, 0x00, screen_size);/* 清屏: 全部设为黑色 */
	/* 随便设置出100个为红点 */
	const double T=2*3.14;//周期
	const int A=100;//振幅
	double x,sin_y,cos_y;
	int start_x,start_y;//绘图的起始点像素坐标
	start_x=0;
	start_y = var.yres/2;
	
	while(1){
		#if 1
		for(i=0;i<var.xres;i++){
			x= (i*2*T)/(var.xres);
			sin_y = A*sin(x);
			cos_y = A*cos(x);
			int w=0;
			for(w=0;w<5;w++){
				
				lcd_put_pixel(start_x+i+w,start_y+sin_y, 0xF800);	//
				//lcd_put_pixel(start_x+i+w,start_y+cos_y, 0xFF0000);
			}
			usleep(1000);
		}
		#endif
		memset(fb_base, 0x0000, screen_size); //绿色 /*  RGB565 : 11111 11111 111111   F800*/
		sleep(1);
		memset(fb_base, 0xFFFF, screen_size); //红色 /* */
		sleep(1);
	}
	munmap(fb_base , screen_size);/** 解除内存映射*/
	close(fd_fb);
	return 0;	
}
#endif

#if 0

//14byte文件头
typedef struct
{
	char cfType[2];  //文件类型，"BM"(0x4D42)
	int  cfSize;     //文件大小（字节）
	int  cfReserved; //保留，值为0
	int  cfoffBits;  //数据区相对于文件头的偏移量（字节）
}__attribute__((packed)) BITMAPFILEHEADER;
//__attribute__((packed))的作用是告诉编译器取消结构在编译过程中的优化对齐

//40byte信息头
typedef struct
{
	char ciSize[4];          //BITMAPFILEHEADER所占的字节数
	int  ciWidth;            //宽度
	int  ciHeight;           //高度
	char ciPlanes[2];        //目标设备的位平面数，值为1
	int  ciBitCount;         //每个像素的位数
	char ciCompress[4];      //压缩说明
	char ciSizeImage[4];     //用字节表示的图像大小，该数据必须是4的倍数
	char ciXPelsPerMeter[4]; //目标设备的水平像素数/米
	char ciYPelsPerMeter[4]; //目标设备的垂直像素数/米
	char ciClrUsed[4];       //位图使用调色板的颜色数
	char ciClrImportant[4];  //指定重要的颜色数，当该域的值等于颜色数时（或者等于0时），表示所有颜色都一样重要
}__attribute__((packed)) BITMAPINFOHEADER;

typedef struct
{
	unsigned char blue;
	unsigned char green;
	unsigned char red;
	unsigned char reserved;
}__attribute__((packed)) PIXEL; //颜色模式RGB



typedef struct
{
	int          fbfd; 
	char         *fbp; 
	unsigned int xres; //水平分辨率
	unsigned int yres;	//垂直分辨率
	unsigned int xres_virtual; //虚拟地址
	unsigned int yres_virtual;
	unsigned int xoffset; //横轴位移
	unsigned int yoffset;
	unsigned int bpp; //像素深度
	unsigned long line_length; //线长度
	unsigned long size; //缓冲区大小 

	struct fb_bitfield red; //
	struct fb_bitfield green;
	struct fb_bitfield blue;
} FB_INFO;

#if 0
//-----------------------------------------------------------------
#define MAX_DESC_NUM    2
#define MAX_LAYER_NUM 	1
struct jzfb_dev {
	unsigned int data_buf[MAX_DESC_NUM][MAX_LAYER_NUM];
	unsigned int num_buf;
	void *buf_addr;

	int width;
	int height;

	unsigned int vid_size;
	unsigned int fb_size;
	int bpp;
	int format;

	struct fb_fix_screeninfo fix_info;
	struct fb_var_screeninfo var_info;

	int fd;
};

#define FB0DEV			"/dev/fb0"

static int jzfb_dev_init(struct jzfb_dev * jzfb_dev)
{
	int ret = 0;
	int i, j;

	jzfb_dev->fd = open(FB0DEV, O_RDWR);
	if (jzfb_dev->fd <= 2) {
		perror("fb0 open error");
		return jzfb_dev->fd;
	}

	/* get framebuffer's var_info */
	if ((ret = ioctl(jzfb_dev->fd, FBIOGET_VSCREENINFO, &jzfb_dev->var_info)) < 0) {
		perror("FBIOGET_VSCREENINFO failed");
		goto err_getinfo;
	}

	/* get framebuffer's fix_info */
	if ((ret = ioctl(jzfb_dev->fd, FBIOGET_FSCREENINFO, &jzfb_dev->fix_info)) < 0) {
		perror("FBIOGET_FSCREENINFO failed");
		goto err_getinfo;
	}

	jzfb_dev->var_info.width = jzfb_dev->var_info.xres;
	jzfb_dev->var_info.height = jzfb_dev->var_info.yres;
	jzfb_dev->bpp = jzfb_dev->var_info.bits_per_pixel >> 3;

	jzfb_dev->width = jzfb_dev->var_info.xres;
	jzfb_dev->height = jzfb_dev->var_info.yres;
	/* format rgb888 use 4 word ; format nv12/nv21 user 2 word */
	jzfb_dev->fb_size = jzfb_dev->var_info.xres * jzfb_dev->var_info.yres * jzfb_dev->bpp;
	jzfb_dev->num_buf = jzfb_dev->var_info.yres_virtual / jzfb_dev->var_info.yres;
	jzfb_dev->vid_size = jzfb_dev->fb_size * jzfb_dev->num_buf;

	jzfb_dev->buf_addr = mmap(0, jzfb_dev->vid_size, PROT_READ | PROT_WRITE, MAP_SHARED, jzfb_dev->fd, 0);
	if(jzfb_dev->buf_addr == 0) {
		perror("Map failed");
		ret = -1;
		goto err_getinfo;
	}

	for(i = 0; i < MAX_DESC_NUM; i++) {
		for(j = 0; j < MAX_LAYER_NUM; j++) {
			jzfb_dev->data_buf[i][j] = (unsigned int)(jzfb_dev->buf_addr +
					j * jzfb_dev->fb_size +
					i * jzfb_dev->fb_size * MAX_LAYER_NUM);
		}
	}
	printf("xres = %d, yres = %d line_length = %d fbsize = %d, num_buf = %d, vidSize = %d\n",
			jzfb_dev->var_info.xres, jzfb_dev->var_info.yres,
			jzfb_dev->fix_info.line_length, jzfb_dev->fb_size,
			jzfb_dev->num_buf, jzfb_dev->vid_size);
	return ret;

err_getinfo:
	close(jzfb_dev->fd);
	return ret;
}
#endif

//构造bmp图片信息结构体
typedef struct
{
	unsigned int width; //图片宽度  
	unsigned int height; //图片高度
	unsigned int bpp;  //图片位数
	unsigned long size; //图片大小
	unsigned int data_offset; 
} IMG_INFO;

FB_INFO fb_info;
IMG_INFO img_info;

int show_bmp(char *img_name);

//
static int cursor_bitmap_format_convert(char *dst,char *src, unsigned long img_len_one_line)
{
	int img_len ,fb_len ,ret;
	char *p;
	__u32 val;
	PIXEL pix;

	p = (char *)&val;

	img_len = img_info.width; /*一行图片的长度*/
	fb_len = fb_info.xres; /*一行显示屏的长度*/

	/*进行x轴的偏移*/
	dst += fb_info.xoffset * (fb_info.bpp / 8);
	fb_len -= fb_info.xoffset;

	/*bmp 数据是上下左右颠倒的，这里只进行左右的处理*/
	/*先定位到图片的最后一个像素的地址，然后往第一个像素的方向处理，进行左右颠倒的处理*/
	src += img_len_one_line - 1;

	/*处理一行要显示的数据*/
	while(1) {
		if (img_info.bpp == 32)
			pix.reserved = *(src--);
		pix.red   = *(src--);
		pix.green = *(src--);
		pix.blue  = *(src--);

		//printf("red: %d, green: %d, blue: %d \n", pix.red, pix.green, pix.blue);

		val = 0x00;
		val |= (pix.red >> (8 - fb_info.red.length)) << fb_info.red.offset;
		val |= (pix.green >> (8 - fb_info.green.length)) << fb_info.green.offset;
		val |= (pix.blue >> (8 - fb_info.blue.length)) << fb_info.blue.offset;


		if (fb_info.bpp == 16) {
			*(dst++) = *(p + 0);
			*(dst++) = *(p + 1);
		}
		else if (fb_info.bpp == 24) {
			*(dst++) = *(p + 0);
			*(dst++) = *(p + 1);
			*(dst++) = *(p + 2);
		}
		else if (fb_info.bpp == 32) {
			*(dst++) = *(p + 0);
			*(dst++) = *(p + 1);
			*(dst++) = *(p + 2);
			*(dst++) = *(p + 3);
		}

		/*超过图片长度或显示屏长度认为一行处理完了*/
		img_len--;
		fb_len--;
		if (img_len <= 0 || fb_len <= 0)
			break;
	}
#if 0
	printf("r = %d\n", pix.red);
	printf("g = %d\n", pix.green);
	printf("b = %d\n", pix.blue);
#endif
	return 0;
}

int show_bmp(char *img_name)
{
	FILE *fp;
	int ret = 0;
	BITMAPFILEHEADER FileHead;
	BITMAPINFOHEADER InfoHead;

	if(img_name == NULL) {
		printf("img_name is null\n");
		return -1;
	}

	//打开文件
	fp = fopen( img_name, "rb" );
	if(fp == NULL) {
		printf("img[%s] open failed\n", img_name);
		ret = -1;
		goto err_showbmp;
	}

	/* 移位到文件头部 */
	fseek(fp, 0, SEEK_SET);

	//读取图片内容
	ret = fread(&FileHead, sizeof(BITMAPFILEHEADER), 1, fp);
	if ( ret != 1) {
		printf("img read failed\n");
		ret = -1;
		goto err_showbmp;
	}

	//检测是否是bmp图像
	if (memcmp(FileHead.cfType, "BM", 2) != 0) {
		printf("it's not a BMP file[%c%c]\n", FileHead.cfType[0], FileHead.cfType[1]);
		ret = -1;
		goto err_showbmp;
	}

	ret = fread( (char *)&InfoHead, sizeof(BITMAPINFOHEADER),1, fp );
	if ( ret != 1) {
		printf("read infoheader error!\n");
		ret = -1;
		goto err_showbmp;
	}

	img_info.width       = InfoHead.ciWidth;
	img_info.height      = InfoHead.ciHeight;
	img_info.bpp         = InfoHead.ciBitCount;
	img_info.size        = FileHead.cfSize;
	img_info.data_offset = FileHead.cfoffBits;

	printf("img info w[%d] h[%d] bpp[%d] size[%ld] offset[%d]\n", img_info.width, img_info.height, img_info.bpp, img_info.size, img_info.data_offset);

	if (img_info.bpp != 16 && img_info.bpp != 24 && img_info.bpp != 32) {
		printf("img bpp is not 24 or 32\n");
		ret = -1;
		goto err_showbmp;
	}


	/*
	 *一行行处理
	 */
	char *buf_img_one_line;
	char *buf_fb_one_line;
	char *p;
	int fb_height;

	long img_len_one_line = img_info.width * (img_info.bpp / 8); //图片的行大小
	long fb_len_one_line = fb_info.line_length; //framebuffer 行大小

	printf("img_len_one_line = %d\n", img_len_one_line);
	printf("fb_len_one_line = %d\n", fb_info.line_length);

	buf_img_one_line = (char *)calloc(1, img_len_one_line + 256);
	if(buf_img_one_line == NULL) {
		printf("alloc failed\n");
		ret = -1;
		goto err_showbmp;
	}

	buf_fb_one_line = (char *)calloc(1, fb_len_one_line + 256);
	if(buf_fb_one_line == NULL) {
		printf("alloc failed\n");
		ret = -1;
		goto err_showbmp;
	}

	fseek(fp, img_info.data_offset, SEEK_SET);

	p = fb_info.fbp + fb_info.yoffset * fb_info.line_length; /*进行y轴的偏移*/
	fb_height = fb_info.yres;
	while (1) {
		memset(buf_img_one_line, 0, img_len_one_line);
		memset(buf_fb_one_line, 0, fb_len_one_line);
		ret = fread(buf_img_one_line, 1, img_len_one_line, fp);
		if (ret < img_len_one_line) {
			/*图片读取完成，则图片显示完成*/
			printf("read to end of img file\n");
			cursor_bitmap_format_convert(buf_fb_one_line, buf_img_one_line, img_len_one_line); /*数据转换*/
			memcpy(fb_info.fbp, buf_fb_one_line, fb_len_one_line);
			break;
		}

		cursor_bitmap_format_convert(buf_fb_one_line, buf_img_one_line, img_len_one_line); /*数据转换*/
		memcpy(p, buf_fb_one_line, fb_len_one_line); /*显示一行*/
		p += fb_len_one_line;
		printf("----------end----------------------");
		/*超过显示屏宽度认为图片显示完成*/
		fb_height--;
		if (fb_height <= 0)
			break;
	}

	free(buf_img_one_line);
	free(buf_fb_one_line);

	fclose(fp);
	return ret;
err_showbmp:
	if (fp)
		fclose(fp);
	return ret;
}


//输入图片文件名
int show_picture(char *img_name)
{
	struct fb_var_screeninfo vinfo; //用于记录用户可以修改的显示控制器的参数
	struct fb_fix_screeninfo finfo; //用于记录用户不能修改的显示控制器的参数

	//判读设备文件句柄
	if (fb_info.fbfd <= -1) {
		printf("fb open fialed\n");
		return -1;
	}

	//获得FrameBuffer设备的参数
	if (ioctl(fb_info.fbfd, FBIOGET_FSCREENINFO, &finfo)) {
		printf("fb ioctl fialed\n");
		return -1;
	}

	//获得FrameBuffer设备的可变参数信息
	if (ioctl(fb_info.fbfd, FBIOGET_VSCREENINFO, &vinfo)) {
		printf("fb ioctl fialed\n");
		return -1;
	}

	//
	fb_info.xres = vinfo.xres;
	fb_info.yres = vinfo.yres;
	fb_info.xres_virtual = vinfo.xres_virtual;
	fb_info.yres_virtual = vinfo.yres_virtual;
	fb_info.xoffset = vinfo.xoffset;
	fb_info.yoffset = vinfo.yoffset;
	fb_info.bpp  = vinfo.bits_per_pixel;
	//fb_info.bpp  = 16;
	fb_info.line_length = finfo.line_length;
	fb_info.size = finfo.smem_len;

	memcpy(&fb_info.red, &vinfo.red, sizeof(struct fb_bitfield));
	memcpy(&fb_info.green, &vinfo.green, sizeof(struct fb_bitfield));
	memcpy(&fb_info.blue, &vinfo.blue, sizeof(struct fb_bitfield));

	printf("fb info x[%d] y[%d] x_v[%d] y_v[%d] xoffset[%d] yoffset[%d] bpp[%d] line_length[%ld] size[%ld]\n", fb_info.xres, fb_info.yres, fb_info.xres_virtual, fb_info.yres_virtual, fb_info.xoffset, fb_info.yoffset, fb_info.bpp, fb_info.line_length, fb_info.size);

	printf("fb info red off[%d] len[%d] msb[%d]\n", fb_info.red.offset, fb_info.red.length, fb_info.red.msb_right);
	printf("fb info green off[%d] len[%d] msb[%d]\n", fb_info.green.offset, fb_info.green.length, fb_info.green.msb_right);
	printf("fb info blue off[%d] len[%d] msb[%d]\n", fb_info.blue.offset, fb_info.blue.length, fb_info.blue.msb_right);

	//检测图片是否合格
	if (fb_info.bpp != 16 && fb_info.bpp != 24 && fb_info.bpp != 32) {
		printf("fb bpp is not 16,24 or 32\n");
		return -1;
	}

	if (fb_info.red.length > 8 || fb_info.green.length > 8 || fb_info.blue.length > 8) {
		printf("fb red|green|blue length is invalid\n");
		return -1;
	}

	// 内存映射
	fb_info.fbp = (char *)mmap(0, fb_info.size, PROT_READ | PROT_WRITE, MAP_SHARED, fb_info.fbfd, 0);
	if (fb_info.fbp == (char *)-1) {
		printf("mmap fialed\n");
		return -1;
	}

	show_bmp(img_name);

	sleep(100);

	//删除映射
	munmap(fb_info.fbp, fb_info.size);

	return 0;
}


int main(int argc, char **argv)
{
	char img_name[64];
	char ret;


	if (argc != 2) {
		printf("arg error\n");
		return 0;
	}

#if 0
	ret = jzfb_dev_init(jzfb_dev);
	if(ret) {
		//IMP_LOG_ERR(TAG,"jzfb_dev init error!\n");
		return ret;
	}

	
	/* Step.1 System init */
	ret = sample_system_init();
	if(ret < 0){
		//IMP_LOG_ERR(TAG, "IMP_System_Init() failed\n");
		return -1;
	}
#endif

	snprintf(img_name, sizeof(img_name), "%s", argv[1]);
	printf("img_name = %s\n", img_name);

	fb_info.fbfd = open("/dev/fb0", O_RDWR); //打开fb设备
	if (!fb_info.fbfd) {
		printf("Error: cannot open framebuffer device(/dev/fb0).\n");
		return -1;
	}
	show_picture(img_name);

	close(fb_info.fbfd);
		return 0;
}

#endif 