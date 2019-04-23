#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/soundcard.h>

int main(int argc,char *argv[])
{
	printf("my music project!\n");
	
	system("madplay Apologize.mp3 &");//新创建一个进程在后台播放，前台的资源与后台无关。
	
	sleep(5);
	
	system("killall -STOP madplay");	//暂停播放
	
	sleep(3);
	
	system("killall -CONT madplay");	//继续播放
	
	sleep(3);
	
	system("killall -KILL madplay");	//停止播放
	
	//printf("after listening...!\n");
	
	return 0;
}