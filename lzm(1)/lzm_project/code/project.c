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

//准备存放映射内存地址的指针
static int *lcd_addr;
//准备存放图片数据的数组
static char bmp_buf[800*480*3]={0};
//存放LCD屏幕文件描述符
static int lcd_fd;
//定义一个存放BMP图片名字的数组
static char *bmp_name[20];
//定义一个存放BMP图片名字的数组
static char *mp3_name[20];
/***
	函数功能：打开LCD屏幕
			  通过访问lcd_addr来修改LCD屏幕颜色值
***/
int open_lcd()
{
	//1.打开LCD屏幕设备文件
	lcd_fd = open("/dev/fb0",O_RDWR);
	//2.判断文件打开是否成功
	if(-1 == lcd_fd)
	{
		printf("open error!\n");
		return -1;
	}
	else
	{
		printf("open success!\n");
	}
	//4.映射LCD屏幕设备文件到内存中
	lcd_addr = mmap(NULL,				//映射区的开始地址，NULL为系统自动分配
				800*480*4,				//映射区的长度（大小/字节）
				PROT_READ|PROT_WRITE,	//内容可以被读取和写入
				MAP_SHARED,				//共享内存
				lcd_fd,					//有效的文件描述符
				0);						//被映射对象的内容起点
				
	return 0;
}

/***
	函数功能：关闭LCD屏幕
***/
void close_lcd()
{
	//关闭文件
	close(lcd_fd);
	
	//解除映射
	munmap(lcd_addr,800*480*4);
}

/****************************************************
 *函数名称:file_size_get
 *输入参数:pfile_path	-文件路径
 *返 回 值:-1		-失败
		   其他值	-文件大小
 *说	明:获取文件大小
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
	函数功能：LCD任意位置绘制图片
	参数：x：x坐标
		  y：y坐标
		  pbmp_path：图片路径
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
	
	/* 申请位图资源，权限可读可写 */	
	bmp_fd=open(pbmp_path,O_RDWR);
	
	if(bmp_fd == -1)
	{
	   printf("open bmp error\r\n");
	   
	   return -1;	
	}
	
	/* 读取位图头部信息 */
	read(bmp_fd,buf,54);
	
	/* 宽度  */
	bmp_width =buf[18];
	bmp_width|=buf[19]<<8;
	//printf("bmp_width=%d\r\n",bmp_width);
	
	/* 高度  */
	bmp_height =buf[22];
	bmp_height|=buf[23]<<8;
	//printf("bmp_height=%d\r\n",bmp_height);	
	
	/* 文件类型 */
	bmp_type =buf[28];
	bmp_type|=buf[29]<<8;
	//printf("bmp_type=%d\r\n",bmp_type);	

	if(middle)
	{
		//居中显示
		/* 设置显示x、y坐标起始结束、结束位置 */
		x_s = (800-bmp_width)/2;
		y_s = (480-bmp_height)/2;
		x_e = (800+bmp_width)/2;
		y_e = (480+bmp_height)/2;
	}
	else
	{
		/* 设置显示x、y坐标结束位置 */
		x_e = x + bmp_width;
		y_e = y + bmp_height;
	}
	
	

	/* 获取位图文件的大小 */
	bmp_size=file_size_get(pbmp_path);
	
	/* 读取所有RGB数据 */
	
	read(bmp_fd,pbmp_buf,bmp_size-54);
	// 将颠倒的图片翻转过来

	if(middle)
	{
		for(y = 0;y < 480; y++)
		{
			for (x = 0;x < 800; x++)
			{
				if(x >= x_s && x < x_e && y >= y_s && y < y_e)
				{
					/* 获取红绿蓝颜色数据 */
					blue  = *pbmp_buf++;
					green = *pbmp_buf++;
					red   = *pbmp_buf++;
					
					/* 判断当前的位图是否32位颜色 */
					if(bmp_type == 32)
					{
						pbmp_buf++;
					}
					
					/* 组成24bit颜色 */
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
					/* 获取红绿蓝颜色数据 */
					blue  = *pbmp_buf++;
					green = *pbmp_buf++;
					red   = *pbmp_buf++;
					
					/* 判断当前的位图是否32位颜色 */
					if(bmp_type == 32)
					{
						pbmp_buf++;
					}
					
					/* 组成24bit颜色 */
					color = red << 16 | green << 8 | blue << 0;
					lcd_addr[(y_e-1-y+y_s)*800+x]=color;			
			}
		}
	}
	
	/* 不再使用BMP，则释放bmp资源 */
	close(bmp_fd);	
	
	return 0;
}
/***
	函数功能：LCD清屏
***/
void lcd_draw_null()
{
	int i;
	for(i=0; i<800*480; i++)
		lcd_addr[i] = 0;
		
}
/***
	函数功能：遍历目录下全部文件
	参数：
		  pbmp_path：目录路径
***/
int show_dir(const char *pdir_path,char *file_name)
{

	DIR *dp;
	struct dirent *ep;
	
	//1.访问目录
	dp = opendir(pdir_path);
	if(dp == NULL)
	{
		perror("opendir error!");
	}
	
	//2.进入目录
	chdir(pdir_path);
	
	//3.读取目录中的每一项
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
	//重置目录指针
	//chdir("-");
	//4.关闭目录
	closedir(dp);
	return 0;
}
/***
	函数功能：获取触摸屏的X,Y坐标
	参数：x：存放x坐标的变量
		  y：存放y坐标的变量
***/
int get_xy(int *x,int *y)
{
	//打开触摸屏文件
	int fd = open("/dev/input/event0",O_RDONLY);
	if(fd < 0)
	{
		perror("open ts error");	//perror专门输出错误信息
		return -1;
	}
	
	//定义读取缓冲区buf
	struct input_event buf;		//struct 结构体类型 结构体变量名
	while(1)
	{
		//读取触摸事件
		read(fd,&buf,sizeof(buf));	//从fd中读取sizeof(buf)个字节的数据存放到buf
		//判断坐标事件，存储坐标
		if(buf.type == EV_ABS && buf.code == ABS_X)
		{
			*x = buf.value;
		}
		if(buf.type == EV_ABS && buf.code == ABS_Y)
		{
			*y = buf.value;
		}
		//判断是松手的时候，输出数据
		if(buf.type == EV_KEY && buf.value == 0)
		{
			break;
		}
			
	}
	
	//关闭文件
	close(fd);
	return 0;
}
/***
	函数功能：音乐播放器（点击屏幕：播放，停止，继续，暂停）
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
//幻灯片
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
	四、图片特效
	参数：
		bmppath：图片名称
		flag：刷图方式
	功能：多种显示图片的特效
*************************************/
int show_slide_bmp(char *bmppath,int flag)
{
	int bmpfd;
	int i,j,n;
	int tmp[480*800]={0};
	// 打开你要显示的bmp图片
	bmpfd=open(bmppath,O_RDWR);
	if(bmpfd==-1)
	{
		perror("图片打开错误!\n");
		return -1;
	} 
	char buf[54];
	/* 读取位图头部信息 */
	read(bmpfd,buf,54);
	/* 宽度  */
	unsigned int bmp_width =buf[18];
	bmp_width|=buf[19]<<8;
	/* 高度  */
	unsigned int bmp_height =buf[22];
	bmp_height|=buf[23]<<8;
	// 读取你打开的那个bmp的颜色值
	int x,y;
	/* 设置显示x、y坐标起始结束、结束位置 */
	unsigned int x_s = (800-bmp_width)/2;
	unsigned int y_s = (480-bmp_height)/2;
	unsigned int x_e = (800+bmp_width)/2;
	unsigned int y_e = (480+bmp_height)/2;
	char *pbmp_buf=bmp_buf;
	/* 获取位图文件的大小 */
	int bmp_size=file_size_get(bmppath);
	
	/* 读取所有RGB数据 */
	unsigned int blue, green, red;
	unsigned int color;
	read(bmpfd,pbmp_buf,bmp_size-54);
	// 将颠倒的图片翻转过来
	for(y = 0;y < 480; y++)
	{
		for (x = 0;x < 800; x++)
		{
			if(x >= x_s && x < x_e && y >= y_s && y < y_e)
			{
				/* 获取红绿蓝颜色数据 */
				blue  = *pbmp_buf++;
				green = *pbmp_buf++;
				red   = *pbmp_buf++;
				
				/* 组成24bit颜色 */
				color = red << 16 | green << 8 | blue << 0;
				tmp[(479-y)*800+x] = color;	
			}
			else
				tmp[y*800+x] = 0;
		}
	}
	//================================
	//左刷
	if(flag == LS)
	{
		for(n=800;n>=0;n-=10)
			for(j=0;j<480;j++)
				for(i=0;i<800-n;i++)	
					*(lcd_addr+j*800+i)=tmp[j*800+i];
	}
	//右刷
	else if(flag == RS)
	{
		for(n=800;n>=0;n-=10)
			for(j=479;j>=0;j--)
				for(i=799;i>=0+n;i--)
					*(lcd_addr+j*800+i)=tmp[j*800+i];
	}
	//从中间放大
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
	// 关闭
	close(bmpfd);

	return 0;
}

/***
	函数功能：相册（点击屏幕：上一张，下一张，幻灯片）
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
	函数功能：目录（点击屏幕：选择功能）
***/
int menu()
{
	int x,y;
	printf("menu\n");
	while(1)
	{
		lcd_draw_bmp(0,0,"./data/image/main/menu.bmp",0);
		get_xy(&x,&y);
		if(x < 80 && y < 50)	//退出
			return 3;
		else if(x > 360 && y > 90 && x < 550 && y < 210)	//相册
			return 1;
		else if(x > 580 && y > 210 && x < 760 && y < 330)	//音乐
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
		//退出主循环（退出程序）
		//例子：if(ret == 3)
		//			break;
	}	
	close_lcd();
	return 0;
}

