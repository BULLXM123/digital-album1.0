#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>

int main()
{
	//打开文件
	int fd = open("/dev/fb0",O_RDWR);
	if(fd == -1)
	{
		printf("open lcd error\n");
		return -1;
	}
	else
	{
		printf("open lcd success\n");
	}
	int *lcd_buf = (int *)mmap(		NULL, 					//映射区的开始地址，设置为NULL时表示由系统决定映射区的起始地址
									800*480*4, 				//映射区的长度
									PROT_READ|PROT_WRITE, 	//内容可以被读取和写入
									MAP_SHARED,				//共享内存
									fd, 					//有效的文件描述符
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
	
	//将屏幕刷成红，绿，蓝（竖刷）
	int x,y;
	for(y=0; y<480; y++)
	{
		for(x=0; x<200; x++)
		{
			lcd_buf[x+y*800] = 0x00ff0000;		
		}
		for(x=200; x<500; x++)
		{
			lcd_buf[x+y*800] = 0x0000ff00;		
		}
		for(x=500; x<800; x++)
		{
			lcd_buf[x+y*800] = 0x000000ff;
		}			
	}
	
	
	//取消映射
	munmap(lcd_buf,800*480*4);
	
	//关闭文件
	close(fd);
	
	return 0;
}