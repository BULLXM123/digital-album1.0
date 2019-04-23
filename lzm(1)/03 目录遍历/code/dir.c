#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdio.h>

//����һ�����BMPͼƬ���ֵ�����
static char *bmp_name[20];
//����һ�����MP3�ļ����ֵ�����
static char *mp3_name[20];

int show_dir(const char *pdir_path,const char *file_name)
{
	DIR *dp;
	struct dirent *ep;
	
	//1.����Ŀ¼
	dp = opendir(pdir_path);
	if(dp == NULL)
	{
		perror("opendir error!");
		return -1;
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
	//4.�ر�Ŀ¼
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




