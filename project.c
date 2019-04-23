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

//准备存放映射内存地址的指针
static int *lcd_addr;
//准备存放图片数据的数组
static char bmp_buf[800*480*3]={0};
//存放LCD屏幕文件描述符
static int lcd_fd;
//定义一个存放BMP图片名字的数组
static char *name[20];
//定义两个变量存放坐标
int x,y;
//定义宏变量
int i,num,temp;

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

	/* 设置显示x、y坐标结束位置 */
	x_e = x + bmp_width;
	y_e = y + bmp_height;
	
	/* 获取位图文件的大小 */
	bmp_size=file_size_get(pbmp_path);
	
	/* 读取所有RGB数据 */
	
	read(bmp_fd,pbmp_buf,bmp_size-54);
	// 将颠倒的图片翻转过来

	
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
	
	/* 不再使用BMP，则释放bmp资源 */
	close(bmp_fd);	
	
	return 0;
}
/***
	函数功能：LCD清屏
***/
void lcd_draw_null()
{
    int j;
	for(j=0; j<800*400; j++)
		lcd_addr[j] = 0;
		
}
/***
	函数功能：遍历目录下全部文件
	参数：
		  pbmp_path：目录路径
***/
int show_dir(const char *pdir_path)
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
	//存储图片张数
	if(name[i]==NULL)
		num = i-1;
	//4.关闭目录
	closedir(dp);
	return 0;
}

/***
	函数功能：幻灯片播放bmp图片
***/
int bmp_run()
{
	//打开目录
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
	函数功能：获取触摸屏的X,Y坐标
	参数：x：存放x坐标的变量
		  y：存放y坐标的变量
***/
int get_xy(int *x,int *y)
{
	//访问触摸屏文件
	int fd = open("/dev/input/event0",O_RDONLY);
	if(fd == -1)
	{
		perror("open ts error");
		return -1;
	}
	//定义事件结构体
	struct input_event buf;
	while(1)
	{
		//读取触摸屏数据
		read(fd,&buf,sizeof(buf));
		//获取X轴坐标
		if(buf.type == EV_ABS && buf.code == ABS_X)
		{
			*x = buf.value;
		}
		//获取Y轴坐标
		if(buf.type == EV_ABS && buf.code == ABS_Y)
		{
			*y = buf.value;
		}
		//松手判断
		if(buf.type == EV_KEY && buf.code == BTN_TOUCH && buf.value == 0)
		{
			break;
		}
	}
	//关闭文件
	close(fd);
	return 0;
}

/***
	函数功能：手动切换bmp图片
***/
int change_bmp()
{
	chdir("./image");
	lcd_draw_bmp(0,0,name[i]);
	printf("this is %s\n",name[i]);
	get_xy(&x,&y);
	//手动切换下一张图片
	if(x>400 && y>440)
	{
		lcd_draw_null();
		i++;
		x=0;
		y=0;
	}
	//手动切换上一张图片
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
	//返回目录
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
	函数功能：目录显示
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
	//调用目录选择函数
	chose();
}

/***
	函数功能：目录选择
***/
int chose()
{
	get_xy(&x,&y);
	//进入手动切换图片模式
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
	//进入幻灯片播放图片模式
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
	函数功能：音乐播放器（点击屏幕：播放，停止，继续，暂停）
***/
int musicplay()
{
	printf("my music project!\n");
	system("madplay Apologize.mp3 &");//新创建一个进程在后台播放，前台的资源与后台无关。
	sleep(5);
	
	system("killall -STOP madplay");	//暂停播放
	
	sleep(3);
	
	system("killall -CONT madplay");	//继续播放
	
	return 0;
}

//主函数
int main(int argc,char *argv[])
{
	//打卡音乐
	musicplay();
	open_lcd();
	show_dir("./image");
	//返回家目录
	chdir("../");
	while(1)
	{
		menu();
	}
	close_lcd();
	return 0;
}