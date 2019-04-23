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
	while(1)
	{
		//读取触摸屏数据
		read(fd,&buf,sizeof(buf));
		//打印数据
		printf("type=0x%x code=0x%x value=%d\n",buf.type,buf.code,buf.value);
	}
	//关闭文件
	close(fd);
	return 0;
}