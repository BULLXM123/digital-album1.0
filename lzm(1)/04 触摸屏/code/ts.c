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
	while(1)
	{
		//��ȡ����������
		read(fd,&buf,sizeof(buf));
		//��ӡ����
		printf("type=0x%x code=0x%x value=%d\n",buf.type,buf.code,buf.value);
	}
	//�ر��ļ�
	close(fd);
	return 0;
}