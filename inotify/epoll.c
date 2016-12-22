#include <sys/epoll.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define MAXCOUNT 500

struct epoll_event mPendingEventItems[10];
char buf[MAXCOUNT];

int main(int argc ,char ** argv)
{
	int mEpollFd;
	int fd;
	int i;
	struct epoll_event eventItem;

	if (argc<2)
	{
		printf("usage: %s <filename1 filename2>\n",argv[0]);
		return -1;
	}


	// 1.init
	mEpollFd = epoll_create(10); 


	// 2. open file & 监听 fd
	for( i=1; i<argc; i++)
	{
		fd = open(argv[i],O_RDWR);
		eventItem.events = EPOLLIN;
		eventItem.data.fd = fd;
		epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &eventItem);
	}

	// 3. wait & process
	while(1)
	{

		int pollResult = epoll_wait(mEpollFd, mPendingEventItems, 10, -1);

		for (i=0; i<pollResult; i++)
		{
			if(mPendingEventItems[i].events == EPOLLIN) // 确保可读事件发生
			{
				memset(buf,0,MAXCOUNT);
				int count = read(mPendingEventItems[i].data.fd,buf,MAXCOUNT);
				buf[count] = '\0';
				printf("have data:%s\n",buf);
			}
		}
	}
}
