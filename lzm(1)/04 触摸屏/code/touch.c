#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/input.h>

int main()
{
	//���ʴ������ļ�
	int fd = open("/dev/input/event0",O_RDONLY);
	if(fd == -1)
	{
		perror("open ts error");
		return -1;
	}
	//�����¼��ṹ��
	struct input_event buf;
	int x,y;
	while(1)
	{
		//��ȡ����������
		read(fd,&buf,sizeof(buf));
		//��ȡX������
		if(buf.type == EV_ABS && buf.code == ABS_X)
		{
			x = buf.value;
		}
		//��ȡY������
		if(buf.type == EV_ABS && buf.code == ABS_Y)
		{
			y = buf.value;
		}
		//�����ж�
		if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
		{
			if(x < 400 && y < 200)
			{
				printf("���Ϸ�\n");
			}
			if(x < 400 && y > 200)
			{
				printf("���·�\n");
			}
			if(x > 400 && y < 200)
			{
				printf("���Ϸ�\n");
			}
			if(x > 400 && y > 200)
			{
				printf("���·�\n");
			}
		}
	}
	//�ر��ļ�
	close(fd);
	return 0;
}
