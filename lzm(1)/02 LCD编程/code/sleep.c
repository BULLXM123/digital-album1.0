#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>

#define FILE_NUM	7
int main(int argc,char *argv[])
{
	//打开LCD文件
	int lcdfd = open("/dev/fb0",O_RDWR);
	if(lcdfd == -1)
	{
		printf("open lcd error\n");
		return -1;
	}
	else
	{
		printf("open lcd success\n");
	}
	//映射LCD屏幕
	int *lcd_buf = (int *)mmap(		NULL, 					//映射区的开始地址，设置为NULL时表示由系统决定映射区的起始地址
									800*480*4, 				//映射区的长度
									PROT_READ|PROT_WRITE, 	//内容可以被读取和写入
									MAP_SHARED,				//共享内存
									lcdfd, 					//有效的文件描述符
									0						//被映射对象内容的起点
								);
	if(lcd_buf == MAP_FAILED)
	{
		printf("mmap error\n");
		return -1;
	}
	else
	{
		printf("mmap success\n");
	}
	
	//定义BMP图片路径指针数组
	char *bmpname[] = {"1.bmp","2.bmp","3.bmp","4.bmp","5.bmp","6.bmp","7.bmp","8.bmp"};
	//定义自变量
	int flag = 0;
	while(1)
	{
		//打开BMP图片文件
		int bmpfd = open(bmpname[flag],O_RDWR);
		if(bmpfd == -1)
		{
			perror("open bmp error\n");
			return -1;
		}
		else
		{
			printf("open bmp success\n");
		}
		printf("this is bmp %d\n",flag+1);
		//定义BMP图片数据缓冲区
		char bmp_buf[800*480*3];
		//跳过54个字节
		lseek(bmpfd,54,SEEK_SET);		//参数：文件描述符，偏移字节数，偏移起始位
		//读取BMP数据
		read(bmpfd,bmp_buf,800*480*3);
		//处理bmp图片数据（4字节合并成3字节），写入到lcd屏幕
		int x,y;
		for(y=0; y<480; y++)
		{
			for(x=0; x<800; x++)
			{
				lcd_buf[(479-y)*800+x] = bmp_buf[(y*800+x)*3+2] << 16 | bmp_buf[(y*800+x)*3+1] << 8 | bmp_buf[(y*800+x)*3] << 0;
			}
		}
		//关闭BMP文件
		close(bmpfd);
		//延迟1秒钟
		sleep(1);
		//自变量自加（切换到下个文件）
		
		flag++;
		if(flag >= sizeof(bmpname)/sizeof(bmpname[0]))
			flag = 0;
	}	
	//取消映射
	munmap(lcd_buf,800*480*4);
	
	//关闭LCD屏幕
	close(lcdfd);
	
	return 0;
}