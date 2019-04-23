#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>

//定义一个存放BMP图片名字的数组
static char *bmp_name[20];
//定义一个存放MP3文件名字的数组
static char *mp3_name[20];

int show_dir(const char *pdir_path,const char *file_name)
{
	DIR *dp;
	struct dirent *ep;
	
	//1.访问目录
	dp = opendir(pdir_path);
	if(dp == NULL)
	{
		perror("opendir error!");
		return -1;
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
	//4.关闭目录
	closedir(dp);
}

int main()
{
	show_dir("./data/image","bmp");
	chdir("../../");
	printf("this is image dir\n");
	sleep(2);
	show_dir("./data/image","mp3");
	printf("this is music dir\n");
}




