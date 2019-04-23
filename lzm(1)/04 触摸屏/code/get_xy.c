#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/input.h>

//��ȡx,y���꣬���������xy����ı���
int get_xy(int *x,int *y)
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
	while(1)
	{
		//��ȡ����������
		read(fd,&buf,sizeof(buf));
		//��ȡX������
		if(buf.type == EV_ABS && buf.code == ABS_X)
		{
			*x = buf.value;
		}
		//��ȡY������
		if(buf.type == EV_ABS && buf.code == ABS_Y)
		{
			*y = buf.value;
		}
		//�����ж�
		if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
		{
			break;
		}
	}
	//�ر��ļ�
	close(fd);
	return 0;
}

int main()
{
	int x,y;
	get_xy(&x,&y);
	printf("x=%d y=%d\n",x,y);
	return 0;
}