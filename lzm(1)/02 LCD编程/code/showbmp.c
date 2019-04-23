#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>

//准备存放映射内存地址的指针
static int *lcd_addr;
//准备存放图片数据的数组
static char bmp_buf[800*480*3]={0};
//存放LCD屏幕文件描述符
static int lcd_fd;

/***
	函数功能：打开LCD屏幕
			  通过访问lcd_addr来修改LCD屏幕颜色值
***/
int open_lcd()
{
	//1.打开LCD屏幕设备文件
	lcd_fd = open("/dev/fb0",O_RDWR);
	//2.判断文件打开是否成功
	if(-1 == lcd_fd)
	{
		printf("open error!\n");
		return -1;
	}
	else
	{
		printf("open success!\n");
	}
	//4.映射LCD屏幕设备文件到内存中
	lcd_addr = mmap(NULL,				//映射区的开始地址，NULL为系统自动分配
				800*480*4,				//映射区的长度（大小/字节）
				PROT_READ|PROT_WRITE,	//内容可以被读取和写入
				MAP_SHARED,				//共享内存
				lcd_fd,					//有效的文件描述符
				0);						//被映射对象的内容起点
				
	return 0;
}

/***
	函数功能：关闭LCD屏幕
***/
void close_lcd()
{
	//关闭文件
	close(lcd_fd);
	
	//解除映射
	munmap(lcd_addr,800*480*4);
}

/****************************************************
 *函数名称:file_size_get
 *输入参数:pfile_path	-文件路径
 *返 回 值:-1		-失败
		   其他值	-文件大小
 *说	明:获取文件大小
 ****************************************************/
unsigned long file_size_get(const char *pfile_path)
{
	unsigned long filesize = -1;	
	struct stat statbuff;
	
	if(stat(pfile_path, &statbuff) < 0)
	{
		return filesize;
	}
	else
	{
		filesize = statbuff.st_size;
	}
	
	return filesize;
}
/***
	函数功能：LCD任意位置绘制图片
	参数：x：x坐标
		  y：y坐标
		  pbmp_path：图片路径
***/
int lcd_draw_bmp(unsigned int x,unsigned int y,const char *pbmp_path)   
{
			 int bmp_fd;
	unsigned int blue, green, red;
	unsigned int color;
	unsigned int bmp_width;
	unsigned int bmp_height;
	unsigned int bmp_type;
	unsigned int bmp_size;
	unsigned int x_s = x;
	unsigned int y_s = y;
	unsigned int x_e ;	
	unsigned int y_e ;
	unsigned char buf[54]={0};
			 char *pbmp_buf=bmp_buf;
	
	/* 申请位图资源，权限可读可写 */	
	bmp_fd=open(pbmp_path,O_RDWR);
	
	if(bmp_fd == -1)
	{
	   printf("open bmp error\r\n");
	   
	   return -1;	
	}
	
	/* 读取位图头部信息 */
	read(bmp_fd,buf,54);
	
	/* 宽度  */
	bmp_width =buf[18];
	bmp_width|=buf[19]<<8;
	printf("bmp_width=%d\r\n",bmp_width);
	
	/* 高度  */
	bmp_height =buf[22];
	bmp_height|=buf[23]<<8;
	printf("bmp_height=%d\r\n",bmp_height);	
	
	/* 文件类型 */
	bmp_type =buf[28];
	bmp_type|=buf[29]<<8;
	//printf("bmp_type=%d\r\n",bmp_type);	

	/* 设置显示x、y坐标结束位置 */
	x_e = x + bmp_width;
	y_e = y + bmp_height;
	
	/* 获取位图文件的大小 */
	bmp_size=file_size_get(pbmp_path);
	
	/* 读取所有RGB数据 */
	
	read(bmp_fd,pbmp_buf,bmp_size-54);
	// 将颠倒的图片翻转过来

	
	for(y = y_s;y < y_e; y++)
	{
		for (x = x_s;x < x_e; x++)
		{
				/* 获取红绿蓝颜色数据 */
				blue  = *pbmp_buf++;
				green = *pbmp_buf++;
				red   = *pbmp_buf++;
				
				/* 判断当前的位图是否32位颜色 */
				if(bmp_type == 32)
				{
					pbmp_buf++;
				}
				
				/* 组成24bit颜色 */
				color = red << 16 | green << 8 | blue << 0;
				lcd_addr[(y_e-1-y+y_s)*800+x]=color;			
		}
		
		
	}
	
	/* 不再使用BMP，则释放bmp资源 */
	close(bmp_fd);	
	
	return 0;
}
//主函数
int main(int argc,char *argv[])
{
	open_lcd();
	
	lcd_draw_bmp(0,0,"1.bmp");
	
	close_lcd();
	return 0;
}