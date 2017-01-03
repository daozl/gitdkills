/**************************************************
 *用inotify机制监视/opt/watch/
 *用epoll机制实现监视inotify_init()返回的文件描述符
 *当监视到/opt/watch/有目录时调用执行箱的脚本
 *Author:zldao
 *Date:2016-12-22
 **************************************************/
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
#include <signal.h>
#include <sys/epoll.h>

#define MAXCOUNT 500
#define MAXEVENTS 3 

/*read监视的fd  执行启动沙箱脚本*/
int process_inotifyevent(int fd)
{
	int count = 0;
	struct inotify_event *event;
	struct inotify_event  *cur;
	char buf[MAXCOUNT];
	int pid;

	count =read(fd,buf,MAXCOUNT);   

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
			if((cur->mask & IN_MOVED_TO) || (cur->mask & IN_CREATE)){
				if(cur->mask & IN_ISDIR){
					
					pid = fork();
					if(pid < 0){
						perror("fork() failed");
						exit(1);
					}
					if(pid == 0){
						execl("/opt/malware_sandbox_deploy/run.sh","./run.sh",cur->name,NULL);
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


void handler(int num)
{
	int status;
	int pid = waitpid(-1,&status,WNOHANG);
	if(WIFEXITED(status)){
		printf("The child %d exit with code %d\n",pid,WEXITSTATUS(status));
	}
}

int main(int argc, char** argv)
{

	int mINotifyFd=0;
	int epollfd=0;
	int nfds,i;
	struct epoll_event epevent,events[MAXEVENTS];

	mINotifyFd = inotify_init();   //初始化，
	inotify_add_watch(mINotifyFd, "/opt/input",IN_MOVED_TO | IN_CREATE);//监听/opt/watch目录下的是否有新的目录和文件

	epollfd = epoll_create1(0);
	if(epollfd < 0){
		perror("epoll_create()");
		exit(1);
	}
	epevent.events = EPOLLIN;
	epevent.data.fd = mINotifyFd;
	if(epoll_ctl(epollfd,EPOLL_CTL_ADD,mINotifyFd,&epevent) < 0){
		perror("epoll_ctl:mINotifyFd");
		exit(1);
	}


	signal(SIGCHLD,handler);

	while(1)
	{
		nfds = epoll_wait(epollfd,events,MAXEVENTS,-1);
		if(nfds < 0){
			if(errno == EINTR)
				continue;
			perror("epoll_wait");
			exit(1);
		}
		for(i = 0; i < nfds; i++){
			if(events[1].data.fd = mINotifyFd){
				process_inotifyevent(mINotifyFd);		
			}else{
				continue;
			}
		}
	}

	return 0;

}
