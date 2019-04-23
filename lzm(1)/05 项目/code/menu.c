#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/input.h>

/***
	函数功能：获取触摸屏的X,Y坐标
	参数：x：存放x坐标的变量
		  y：存放y坐标的变量
***/
int get_xy(int *x,int *y)
{
	//打开触摸屏文件
	int fd = open("/dev/input/event0",O_RDONLY);
	if(fd < 0)
	{
		perror("open ts error");	//perror专门输出错误信息
		return -1;
	}
	
	//定义读取缓冲区buf
	struct input_event buf;		//struct 结构体类型 结构体变量名
	while(1)
	{
		//读取触摸事件
		read(fd,&buf,sizeof(buf));	//从fd中读取sizeof(buf)个字节的数据存放到buf
		//判断坐标事件，存储坐标
		if(buf.type == EV_ABS && buf.code == ABS_X)
		{
			*x = buf.value;
		}
		if(buf.type == EV_ABS && buf.code == ABS_Y)
		{
			*y = buf.value;
		}
		//判断是松手的时候，输出数据
		if(buf.type == EV_KEY && buf.value == 0)
		{
			break;
		}
			
	}
	
	//关闭文件
	close(fd);
	return 0;
}

int menu()
{

	int x,y;
	printf("this is menu\n");
	while(1)
	{
		get_xy(&x,&y);
		if(x < 100 && y < 50)//exit
			return 3;
		if(x < 400)//fun1
			return 1;
		if(x > 400)//fun2
			return 2;
	}

}
void fun1()
{
	int x,y;
	printf("this is fun1\n");
	while(1)
	{
		get_xy(&x,&y);
		if(x < 100 && y < 50)
			break;
	}
}
void fun2()
{
	int x,y;
	printf("this is fun2\n");
	while(1)
	{
		get_xy(&x,&y);
		if(x < 100 && y < 50)
			break;
	}
}
int main()
{
	int ret=0;
	while(1)
	{
		//目录
		ret = menu();
		switch(ret)
		{
			case 1:fun1();break;
			case 2:fun2();break;
		}
		//退出主程序,exit
		if(ret == 3)
		{
			printf("exit\n");
			break;
		}
	}
	
	return 0;
}

