#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>

#define FILE_NUM	7
int main(int argc,char *argv[])
{
	//��LCD�ļ�
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
	//ӳ��LCD��Ļ
	int *lcd_buf = (int *)mmap(		NULL, 					//ӳ�����Ŀ�ʼ��ַ������ΪNULLʱ��ʾ��ϵͳ����ӳ��������ʼ��ַ
									800*480*4, 				//ӳ�����ĳ���
									PROT_READ|PROT_WRITE, 	//���ݿ��Ա���ȡ��д��
									MAP_SHARED,				//�����ڴ�
									lcdfd, 					//��Ч���ļ�������
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
	
	//����BMPͼƬ·��ָ������
	char *bmpname[] = {"1.bmp","2.bmp","3.bmp","4.bmp","5.bmp","6.bmp","7.bmp","8.bmp"};
	//�����Ա���
	int flag = 0;
	while(1)
	{
		//��BMPͼƬ�ļ�
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
		//����BMPͼƬ���ݻ�����
		char bmp_buf[800*480*3];
		//����54���ֽ�
		lseek(bmpfd,54,SEEK_SET);		//�������ļ���������ƫ���ֽ�����ƫ����ʼλ
		//��ȡBMP����
		read(bmpfd,bmp_buf,800*480*3);
		//����bmpͼƬ���ݣ�4�ֽںϲ���3�ֽڣ���д�뵽lcd��Ļ
		int x,y;
		for(y=0; y<480; y++)
		{
			for(x=0; x<800; x++)
			{
				lcd_buf[(479-y)*800+x] = bmp_buf[(y*800+x)*3+2] << 16 | bmp_buf[(y*800+x)*3+1] << 8 | bmp_buf[(y*800+x)*3] << 0;
			}
		}
		//�ر�BMP�ļ�
		close(bmpfd);
		//�ӳ�1����
		sleep(1);
		//�Ա����Լӣ��л����¸��ļ���
		
		flag++;
		if(flag >= sizeof(bmpname)/sizeof(bmpname[0]))
			flag = 0;
	}	
	//ȡ��ӳ��
	munmap(lcd_buf,800*480*4);
	
	//�ر�LCD��Ļ
	close(lcdfd);
	
	return 0;
}