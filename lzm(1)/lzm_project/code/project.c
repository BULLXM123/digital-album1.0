#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/mman.h>
#include <linux/input.h>

#define Middle 1

enum show_bmp{L=1,R,LS,RS,M}show;

//׼�����ӳ���ڴ��ַ��ָ��
static int *lcd_addr;
//׼�����ͼƬ���ݵ�����
static char bmp_buf[800*480*3]={0};
//���LCD��Ļ�ļ�������
static int lcd_fd;
//����һ�����BMPͼƬ���ֵ�����
static char *bmp_name[20];
//����һ�����BMPͼƬ���ֵ�����
static char *mp3_name[20];
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
int lcd_draw_bmp(unsigned int x,unsigned int y,const char *pbmp_path,int middle)   
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

	if(middle)
	{
		//������ʾ
		/* ������ʾx��y������ʼ����������λ�� */
		x_s = (800-bmp_width)/2;
		y_s = (480-bmp_height)/2;
		x_e = (800+bmp_width)/2;
		y_e = (480+bmp_height)/2;
	}
	else
	{
		/* ������ʾx��y�������λ�� */
		x_e = x + bmp_width;
		y_e = y + bmp_height;
	}
	
	

	/* ��ȡλͼ�ļ��Ĵ�С */
	bmp_size=file_size_get(pbmp_path);
	
	/* ��ȡ����RGB���� */
	
	read(bmp_fd,pbmp_buf,bmp_size-54);
	// ���ߵ���ͼƬ��ת����

	if(middle)
	{
		for(y = 0;y < 480; y++)
		{
			for (x = 0;x < 800; x++)
			{
				if(x >= x_s && x < x_e && y >= y_s && y < y_e)
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
					lcd_addr[(479-y)*800+x] = color;	
				}
				else
					lcd_addr[y*800+x] = 0;
			}
		}
	}
	else
	{
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
	int i;
	for(i=0; i<800*480; i++)
		lcd_addr[i] = 0;
		
}
/***
	�������ܣ�����Ŀ¼��ȫ���ļ�
	������
		  pbmp_path��Ŀ¼·��
***/
int show_dir(const char *pdir_path,char *file_name)
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
	int i = 0;
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
		if(strcmp(file_name,"mp3") == 0)
		{
			mp3_name[i++] = ep->d_name;
			printf("%s\n",ep->d_name);
		}
		if(strcmp(file_name,"bmp") == 0)
		{
			bmp_name[i++] = ep->d_name;
			printf("%s\n",ep->d_name);
		}
	}
	//����Ŀ¼ָ��
	//chdir("-");
	//4.�ر�Ŀ¼
	closedir(dp);
	return 0;
}
/***
	�������ܣ���ȡ��������X,Y����
	������x�����x����ı���
		  y�����y����ı���
***/
int get_xy(int *x,int *y)
{
	//�򿪴������ļ�
	int fd = open("/dev/input/event0",O_RDONLY);
	if(fd < 0)
	{
		perror("open ts error");	//perrorר�����������Ϣ
		return -1;
	}
	
	//�����ȡ������buf
	struct input_event buf;		//struct �ṹ������ �ṹ�������
	while(1)
	{
		//��ȡ�����¼�
		read(fd,&buf,sizeof(buf));	//��fd�ж�ȡsizeof(buf)���ֽڵ����ݴ�ŵ�buf
		//�ж������¼����洢����
		if(buf.type == EV_ABS && buf.code == ABS_X)
		{
			*x = buf.value;
		}
		if(buf.type == EV_ABS && buf.code == ABS_Y)
		{
			*y = buf.value;
		}
		//�ж������ֵ�ʱ���������
		if(buf.type == EV_KEY && buf.value == 0)
		{
			break;
		}
			
	}
	
	//�ر��ļ�
	close(fd);
	return 0;
}
/***
	�������ܣ����ֲ������������Ļ�����ţ�ֹͣ����������ͣ��
***/
int musicplay()
{
	int x,y,i=0,play_flag=0;
	char order[50]={0};
	printf("musicplay\n");
	show_dir("./data/music","mp3");
	while(1)
	{
		lcd_draw_bmp(0,0,"../image/main/music.bmp",0);
		get_xy(&x,&y);
		if(x < 80 && y < 50)
		{
			system("killall -KILL madplay");
			chdir("../../");
			break;
		}
		if(x > 210 && y > 230 && x < 280 && y < 315)
		{
			if(i == 0)
				printf("this is first song\n");
			else
			{
				i--;	
				system("killall -KILL madplay");
				printf("prev\n");
				sprintf(order,"madplay %s &",mp3_name[i]);
				system(order);
				sprintf(order,"playing song:%s\n",mp3_name[i]);
				printf(order);
				play_flag = 1;
			}
		}
		if(x > 700 && y > 230 && x < 770 && y < 315)
		{
			if(mp3_name[i+1] == NULL)
				printf("this is last song\n");
			else
			{
				i++;
				system("killall -KILL madplay");
				printf("next\n");
				sprintf(order,"madplay %s &",mp3_name[i]);
				system(order);
				sprintf(order,"playing song:%s\n",mp3_name[i]);
				printf(order);
				play_flag = 1;
			}
		}
		if(x > 375 && y > 230 && x < 460 && y < 315)
		{
			if(play_flag)
			{	
				system("killall -CONT madplay");
				printf("continue\n");
			}
			else
			{
				sprintf(order,"madplay %s &",mp3_name[i]);
				system(order);
				sprintf(order,"playing song:%s\n",mp3_name[i]);
				printf(order);
				play_flag = 1;
			}
		}
		if(x > 530 && y > 230 && x < 610 && y < 315)
		{
			system("killall -STOP madplay");
			printf("pause\n");
		}
		if(x > 20 && y > 230 && x < 120 && y < 315)
		{
			system("killall -KILL madplay");
			printf("stop\n");
			play_flag = 0;
		}
		
	}
}
//�õ�Ƭ
void slide(int i)
{
	int n = i;
	while(1)
	{
		printf("this is NO.%d bmp\n",i+1);
		//lcd_draw_null();
		lcd_draw_bmp(0,0,bmp_name[i++],Middle);
		sleep(1);
		if(bmp_name[i+1]==NULL)
			i = 0;
		if(n == i)
			break;
	}
}
/*************************************
	�ġ�ͼƬ��Ч
	������
		bmppath��ͼƬ����
		flag��ˢͼ��ʽ
	���ܣ�������ʾͼƬ����Ч
*************************************/
int show_slide_bmp(char *bmppath,int flag)
{
	int bmpfd;
	int i,j,n;
	int tmp[480*800]={0};
	// ����Ҫ��ʾ��bmpͼƬ
	bmpfd=open(bmppath,O_RDWR);
	if(bmpfd==-1)
	{
		perror("ͼƬ�򿪴���!\n");
		return -1;
	} 
	char buf[54];
	/* ��ȡλͼͷ����Ϣ */
	read(bmpfd,buf,54);
	/* ���  */
	unsigned int bmp_width =buf[18];
	bmp_width|=buf[19]<<8;
	/* �߶�  */
	unsigned int bmp_height =buf[22];
	bmp_height|=buf[23]<<8;
	// ��ȡ��򿪵��Ǹ�bmp����ɫֵ
	int x,y;
	/* ������ʾx��y������ʼ����������λ�� */
	unsigned int x_s = (800-bmp_width)/2;
	unsigned int y_s = (480-bmp_height)/2;
	unsigned int x_e = (800+bmp_width)/2;
	unsigned int y_e = (480+bmp_height)/2;
	char *pbmp_buf=bmp_buf;
	/* ��ȡλͼ�ļ��Ĵ�С */
	int bmp_size=file_size_get(bmppath);
	
	/* ��ȡ����RGB���� */
	unsigned int blue, green, red;
	unsigned int color;
	read(bmpfd,pbmp_buf,bmp_size-54);
	// ���ߵ���ͼƬ��ת����
	for(y = 0;y < 480; y++)
	{
		for (x = 0;x < 800; x++)
		{
			if(x >= x_s && x < x_e && y >= y_s && y < y_e)
			{
				/* ��ȡ��������ɫ���� */
				blue  = *pbmp_buf++;
				green = *pbmp_buf++;
				red   = *pbmp_buf++;
				
				/* ���24bit��ɫ */
				color = red << 16 | green << 8 | blue << 0;
				tmp[(479-y)*800+x] = color;	
			}
			else
				tmp[y*800+x] = 0;
		}
	}
	//================================
	//��ˢ
	if(flag == LS)
	{
		for(n=800;n>=0;n-=10)
			for(j=0;j<480;j++)
				for(i=0;i<800-n;i++)	
					*(lcd_addr+j*800+i)=tmp[j*800+i];
	}
	//��ˢ
	else if(flag == RS)
	{
		for(n=800;n>=0;n-=10)
			for(j=479;j>=0;j--)
				for(i=799;i>=0+n;i--)
					*(lcd_addr+j*800+i)=tmp[j*800+i];
	}
	//���м�Ŵ�
	else if(flag == M)
	{
		for(n=100;n>=1;n-=1)
		{
			usleep(100);
			for(j=0;j<480/n;j++)
				for(i=0;i<800/n;i++)
					lcd_addr[800*(j+(480-480/n)/2)+i+(800-800/n)/2] = tmp[n*(j*800+i)] ;
		}
	}	
	else if(flag == L)
	{
		for(n=800;n>=0;n-=10)
			for(j=0;j<480;j++)
				for(i=0;i<800-n;i++)
					lcd_addr[800*j+i+n] = tmp[j*800+i];
	}
	else if(flag == R)
	{
		for(n=800;n>=0;n-=10)
			for(j=0;j<480;j++)
				for(i=0;i<800-n;i++)
					lcd_addr[800*j+i] = tmp[j*800+i+n] ;
	}
	// �ر�
	close(bmpfd);

	return 0;
}

/***
	�������ܣ���ᣨ�����Ļ����һ�ţ���һ�ţ��õ�Ƭ��
***/
int album()
{
	int x,y,i=0;
	printf("album\n");
	show_dir("./data/image","bmp");
	int num=rand()%5+1;
	show_slide_bmp(bmp_name[i],num);
	lcd_draw_bmp(0,0,"./main/quit.bmp",0);
	lcd_draw_bmp(0,430,"./main/prev.bmp",0);
	lcd_draw_bmp(720,430,"./main/next.bmp",0);
	lcd_draw_bmp(720,0,"./main/slide.bmp",0);
	while(1)
	{
		num = rand()%5+1;
		get_xy(&x,&y);
		if(x < 80 && y < 50)
		{
			chdir("../../");
			break;
		}
		if(x < 80 && y > 430)
		{
			if(i == 0)
			{
				printf("this is first picture\n");
				continue;
			}
			else
				i--;
			printf("this is NO.%d bmp\n",i+1);
			show_slide_bmp(bmp_name[i],num);
			//lcd_draw_bmp(0,0,bmp_name[i],Middle);
			lcd_draw_bmp(0,0,"./main/quit.bmp",0);
			lcd_draw_bmp(0,430,"./main/prev.bmp",0);
			lcd_draw_bmp(720,430,"./main/next.bmp",0);
			lcd_draw_bmp(720,0,"./main/slide.bmp",0);		
		}
		if(x > 720 && y > 430)
		{
			if(bmp_name[i+2] == NULL)
			{
				printf("this is last picture\n");
				continue;
			}
			else
				i++;
			printf("this is NO.%d bmp\n",i+1);
			show_slide_bmp(bmp_name[i],num);
			//lcd_draw_bmp(0,0,bmp_name[i],Middle);
			lcd_draw_bmp(0,0,"./main/quit.bmp",0);
			lcd_draw_bmp(0,430,"./main/prev.bmp",0);
			lcd_draw_bmp(720,430,"./main/next.bmp",0);
			lcd_draw_bmp(720,0,"./main/slide.bmp",0);
		}
		if(x > 720 && y < 50)
		{
			slide(i);	
		}
	}
}

/***
	�������ܣ�Ŀ¼�������Ļ��ѡ���ܣ�
***/
int menu()
{
	int x,y;
	printf("menu\n");
	while(1)
	{
		lcd_draw_bmp(0,0,"./data/image/main/menu.bmp",0);
		get_xy(&x,&y);
		if(x < 80 && y < 50)	//�˳�
			return 3;
		else if(x > 360 && y > 90 && x < 550 && y < 210)	//���
			return 1;
		else if(x > 580 && y > 210 && x < 760 && y < 330)	//����
			return 2;
	}
}

int main()
{
	open_lcd();
	int ret;
	lcd_draw_bmp(0,0,"./data/image/main/welcome.bmp",0);
	printf("welcome\n");
	//sleep(2);
	while(1)
	{
		ret = menu();
		switch(ret)
		{
			case 1:album();break;
			case 2:musicplay();break;
		}
		if(ret == 3)
		{
			lcd_draw_bmp(0,0,"./data/image/main/bye.bmp",0);
			printf("goodbye\n");
			sleep(2);
			lcd_draw_null();
			break;
		}
		//�˳���ѭ�����˳�����
		//���ӣ�if(ret == 3)
		//			break;
	}	
	close_lcd();
	return 0;
}

