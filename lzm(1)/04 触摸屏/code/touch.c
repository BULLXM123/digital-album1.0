#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/input.h>

int main()
{
	//访问触摸屏文件
	int fd = open("/dev/input/event0",O_RDONLY);
	if(fd == -1)
	{
		perror("open ts error");
		return -1;
	}
	//定义事件结构体
	struct input_event buf;
	int x,y;
	while(1)
	{
		//读取触摸屏数据
		read(fd,&buf,sizeof(buf));
		//获取X轴坐标
		if(buf.type == EV_ABS && buf.code == ABS_X)
		{
			x = buf.value;
		}
		//获取Y轴坐标
		if(buf.type == EV_ABS && buf.code == ABS_Y)
		{
			y = buf.value;
		}
		//松手判断
		if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
		{
			if(x < 400 && y < 200)
			{
				printf("左上方\n");
			}
			if(x < 400 && y > 200)
			{
				printf("左下方\n");
			}
			if(x > 400 && y < 200)
			{
				printf("右上方\n");
			}
			if(x > 400 && y > 200)
			{
				printf("右下方\n");
			}
		}
	}
	//关闭文件
	close(fd);
	return 0;
}
