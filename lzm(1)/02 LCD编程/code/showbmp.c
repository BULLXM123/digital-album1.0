#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>

//׼�����ӳ���ڴ��ַ��ָ��
static int *lcd_addr;
//׼�����ͼƬ���ݵ�����
static char bmp_buf[800*480*3]={0};
//���LCD��Ļ�ļ�������
static int lcd_fd;

/***
	�������ܣ���LCD��Ļ
			  ͨ������lcd_addr���޸�LCD��Ļ��ɫֵ
***/
int open_lcd()
{
	//1.��LCD��Ļ�豸�ļ�
	lcd_fd = open("/dev/fb0",O_RDWR);
	//2.�ж��ļ����Ƿ�ɹ�
	if(-1 == lcd_fd)
	{
		printf("open error!\n");
		return -1;
	}
	else
	{
		printf("open success!\n");
	}
	//4.ӳ��LCD��Ļ�豸�ļ����ڴ���
	lcd_addr = mmap(NULL,				//ӳ�����Ŀ�ʼ��ַ��NULLΪϵͳ�Զ�����
				800*480*4,				//ӳ�����ĳ��ȣ���С/�ֽڣ�
				PROT_READ|PROT_WRITE,	//���ݿ��Ա���ȡ��д��
				MAP_SHARED,				//�����ڴ�
				lcd_fd,					//��Ч���ļ�������
				0);						//��ӳ�������������
				
	return 0;
}

/***
	�������ܣ��ر�LCD��Ļ
***/
void close_lcd()
{
	//�ر��ļ�
	close(lcd_fd);
	
	//���ӳ��
	munmap(lcd_addr,800*480*4);
}

/****************************************************
 *��������:file_size_get
 *�������:pfile_path	-�ļ�·��
 *�� �� ֵ:-1		-ʧ��
		   ����ֵ	-�ļ���С
 *˵	��:��ȡ�ļ���С
 ****************************************************/
unsigned long file_size_get(const char *pfile_path)
{
	unsigned long filesize = -1;	
	struct stat statbuff;
	
	if(stat(pfile_path, &statbuff) < 0)
	{
		return filesize;
	}
	else
	{
		filesize = statbuff.st_size;
	}
	
	return filesize;
}
/***
	�������ܣ�LCD����λ�û���ͼƬ
	������x��x����
		  y��y����
		  pbmp_path��ͼƬ·��
***/
int lcd_draw_bmp(unsigned int x,unsigned int y,const char *pbmp_path)   
{
			 int bmp_fd;
	unsigned int blue, green, red;
	unsigned int color;
	unsigned int bmp_width;
	unsigned int bmp_height;
	unsigned int bmp_type;
	unsigned int bmp_size;
	unsigned int x_s = x;
	unsigned int y_s = y;
	unsigned int x_e ;	
	unsigned int y_e ;
	unsigned char buf[54]={0};
			 char *pbmp_buf=bmp_buf;
	
	/* ����λͼ��Դ��Ȩ�޿ɶ���д */	
	bmp_fd=open(pbmp_path,O_RDWR);
	
	if(bmp_fd == -1)
	{
	   printf("open bmp error\r\n");
	   
	   return -1;	
	}
	
	/* ��ȡλͼͷ����Ϣ */
	read(bmp_fd,buf,54);
	
	/* ���  */
	bmp_width =buf[18];
	bmp_width|=buf[19]<<8;
	printf("bmp_width=%d\r\n",bmp_width);
	
	/* �߶�  */
	bmp_height =buf[22];
	bmp_height|=buf[23]<<8;
	printf("bmp_height=%d\r\n",bmp_height);	
	
	/* �ļ����� */
	bmp_type =buf[28];
	bmp_type|=buf[29]<<8;
	//printf("bmp_type=%d\r\n",bmp_type);	

	/* ������ʾx��y�������λ�� */
	x_e = x + bmp_width;
	y_e = y + bmp_height;
	
	/* ��ȡλͼ�ļ��Ĵ�С */
	bmp_size=file_size_get(pbmp_path);
	
	/* ��ȡ����RGB���� */
	
	read(bmp_fd,pbmp_buf,bmp_size-54);
	// ���ߵ���ͼƬ��ת����

	
	for(y = y_s;y < y_e; y++)
	{
		for (x = x_s;x < x_e; x++)
		{
				/* ��ȡ��������ɫ���� */
				blue  = *pbmp_buf++;
				green = *pbmp_buf++;
				red   = *pbmp_buf++;
				
				/* �жϵ�ǰ��λͼ�Ƿ�32λ��ɫ */
				if(bmp_type == 32)
				{
					pbmp_buf++;
				}
				
				/* ���24bit��ɫ */
				color = red << 16 | green << 8 | blue << 0;
				lcd_addr[(y_e-1-y+y_s)*800+x]=color;			
		}
		
		
	}
	
	/* ����ʹ��BMP�����ͷ�bmp��Դ */
	close(bmp_fd);	
	
	return 0;
}
//������
int main(int argc,char *argv[])
{
	open_lcd();
	
	lcd_draw_bmp(0,0,"1.bmp");
	
	close_lcd();
	return 0;
}