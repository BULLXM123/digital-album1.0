#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

int main()
{
	//���ļ�
	int fd = open("/dev/fb0",O_WRONLY);
	if(fd == -1)
	{
		printf("open lcd error\n");
		return -1;
	}
	else
	{
		printf("open lcd success\n");
	}
	//�ر��ļ�
	close(fd);
	
	return 0;
}