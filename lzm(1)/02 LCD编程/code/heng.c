#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>

int main()
{
	//���ļ�
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
	int *lcd_buf = (int *)mmap(		NULL, 					//ӳ�����Ŀ�ʼ��ַ������ΪNULLʱ��ʾ��ϵͳ����ӳ��������ʼ��ַ
									800*480*4, 				//ӳ�����ĳ���
									PROT_READ|PROT_WRITE, 	//���ݿ��Ա���ȡ��д��
									MAP_SHARED,				//�����ڴ�
									fd, 					//��Ч���ļ�������
									0						//��ӳ��������ݵ����
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
	
	//����Ļˢ�ɺ죬�̣�������ˢ��
	int i;
	for(i=0; i<800*160; i++)
	{
		lcd_buf[i] = 0x00ff0000;		
	}
	for(i=800*160; i<800*320; i++)
	{
		lcd_buf[i] = 0x0000ff00;		
	}
	for(i=800*320; i<800*480; i++)
	{
		lcd_buf[i] = 0x000000ff;		
	}
	
	
	//ȡ��ӳ��
	munmap(lcd_buf,800*480*4);
	
	//�ر��ļ�
	close(fd);
	
	return 0;
}