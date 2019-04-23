#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/soundcard.h>

//׼�����ӳ���ڴ��ַ��ָ��
static int *lcd_addr;
//׼�����ͼƬ���ݵ�����
static char bmp_buf[800*480*3]={0};
//���LCD��Ļ�ļ�������
static int lcd_fd;
//����һ�����BMPͼƬ���ֵ�����
static char *name[20];
//�������������������
int x,y;
//��������
int i,num,temp;

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
	//printf("bmp_width=%d\r\n",bmp_width);
	
	/* �߶�  */
	bmp_height =buf[22];
	bmp_height|=buf[23]<<8;
	//printf("bmp_height=%d\r\n",bmp_height);	
	
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
/***
	�������ܣ�LCD����
***/
void lcd_draw_null()
{
    int j;
	for(j=0; j<800*400; j++)
		lcd_addr[j] = 0;
		
}
/***
	�������ܣ�����Ŀ¼��ȫ���ļ�
	������
		  pbmp_path��Ŀ¼·��
***/
int show_dir(const char *pdir_path)
{

	DIR *dp;
	struct dirent *ep;
	
	//1.����Ŀ¼
	dp = opendir(pdir_path);
	if(dp == NULL)
	{
		perror("opendir error!");
	}
	
	//2.����Ŀ¼
	chdir(pdir_path);
	
	//3.��ȡĿ¼�е�ÿһ��
	i = 0;
	while(1)
	{
		ep = readdir(dp);
		
		if(ep == NULL)
		{
			break;
		}
		if(ep->d_name[0] == '.')
		{
			continue;
		}
	
		name[i++] = ep->d_name;
	}
	
	for(i=0; name[i]!=NULL; i++)
	{
		printf("%s\n",name[i]);
	}
	//�洢ͼƬ����
	if(name[i]==NULL)
		num = i-1;
	//4.�ر�Ŀ¼
	closedir(dp);
	return 0;
}

/***
	�������ܣ��õ�Ƭ����bmpͼƬ
***/
int bmp_run()
{
	//��Ŀ¼
	chdir("./image");
	lcd_draw_bmp(0,0,name[i]);
	printf("this is %s\n",name[i]);
	sleep(5);
	i++;
	lcd_draw_null();
	if(name[i]==NULL)
		i = 0;
}

/***
	�������ܣ���ȡ��������X,Y����
	������x�����x����ı���
		  y�����y����ı���
***/
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

/***
	�������ܣ��ֶ��л�bmpͼƬ
***/
int change_bmp()
{
	chdir("./image");
	lcd_draw_bmp(0,0,name[i]);
	printf("this is %s\n",name[i]);
	get_xy(&x,&y);
	//�ֶ��л���һ��ͼƬ
	if(x>400 && y>440)
	{
		lcd_draw_null();
		i++;
		x=0;
		y=0;
	}
	//�ֶ��л���һ��ͼƬ
	if(x>0 && x<400 && y>440)
	{
		lcd_draw_null();
		if(i>=0)
		    i--;
		x=0;
		y=0;
		if(i<0)
		{
			i=num;
		}
	}
	//����Ŀ¼
	if(x>600 && y>400 && y<440)
	{
		x=0;
		y=0;
		chdir("../");
		temp=1;
	}
	
	if(name[i]==NULL)
		i = 0;
}

/***
	�������ܣ�Ŀ¼��ʾ
***/
int menu()
{
	i = 0;
	lcd_draw_bmp(0,0,"./p1.bmp");
	lcd_draw_bmp(400,0,"./p2.bmp");
	lcd_draw_bmp(0,400,"./stop.bmp");
	lcd_draw_bmp(300,400,"./goon.bmp");
	lcd_draw_bmp(600,400,"./back.bmp");
	lcd_draw_bmp(0,440,"./prev.bmp");
	lcd_draw_bmp(400,440,"./next.bmp");
	//����Ŀ¼ѡ����
	chose();
}

/***
	�������ܣ�Ŀ¼ѡ��
***/
int chose()
{
	get_xy(&x,&y);
	//�����ֶ��л�ͼƬģʽ
	if(x>0 && x<400 && y>0 && y<400)
	{
		x=0;
		y=0;
		while(1)
		{
			change_bmp();
			if(temp==1)
			{
				temp = 0;
				break;
			}
		}
	}
	//����õ�Ƭ����ͼƬģʽ
	if(x>400 && y>0 && y<400)
	{
		x=0;
		y=0;
		while(1)
		{
			bmp_run();
			if(temp==1)
			{
				temp = 0;
				break;
			}
		}
	}
}

/***
	�������ܣ����ֲ������������Ļ�����ţ�ֹͣ����������ͣ��
***/
int musicplay()
{
	printf("my music project!\n");
	system("madplay Apologize.mp3 &");//�´���һ�������ں�̨���ţ�ǰ̨����Դ���̨�޹ء�
	sleep(5);
	
	system("killall -STOP madplay");	//��ͣ����
	
	sleep(3);
	
	system("killall -CONT madplay");	//��������
	
	return 0;
}

//������
int main(int argc,char *argv[])
{
	//������
	musicplay();
	open_lcd();
	show_dir("./image");
	//���ؼ�Ŀ¼
	chdir("../");
	while(1)
	{
		menu();
	}
	close_lcd();
	return 0;
}