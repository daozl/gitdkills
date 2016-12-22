#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <memory.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/inotify.h>
#include <sys/wait.h>

#define MAXCOUNT 500

// 3. read & process
int process_inotifyevent(int fd)
{
	int count = 0;
	struct inotify_event *event;
	struct inotify_event  *cur;
	char buf[MAXCOUNT];
	int pid;


	count =read(fd,buf,MAXCOUNT);
	
	//run to this;

	// 读取出来的数据量小于一个event事件，error
	if(count < sizeof(*event))
	{
		printf("error event\n");
		return -1;
	}

	cur = (struct inotify_event  *) buf;

	while(count >= sizeof(*event))
	{

		if(cur->len > 0)  // name 长度
		{
			printf("have event\n");
			if(cur->mask & IN_MOVED_TO){
				if(cur->mask & IN_ISDIR){
					pid = fork();
					if(pid < 0){
						perror("fork() failed");
						exit(1);
					}
					if(pid == 0){
						execl("./run.sh","./run.sh",cur->name,NULL);
						perror("execl() failed");
						exit(1);
					}
				}
				else{
					printf("only dirctory be need\n");	
				}
			}
		}

		else
			printf("no  event\n");
		
		count -=  sizeof(*event);
		cur += 1;
	}

	return 0;
}


int main(int argc, char** argv)
{
	int mINotifyFd=0;
	// 1
	mINotifyFd = inotify_init();
	// 2
	inotify_add_watch(mINotifyFd, "/opt/watch",IN_MOVED_TO);//监听xxx目录下的 move 、事件是否为目录

	while(1)
	{
		process_inotifyevent(mINotifyFd);
	}

	return 0;

}
