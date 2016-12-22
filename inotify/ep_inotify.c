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
#include <sys/epoll.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>


#define MAX_FILES 1000
#define EPOLL_COUNT 20
#define MAXCOUNT 500
static char *epoll_files[MAX_FILES];

static struct epoll_event mPendingEventItems[EPOLL_COUNT];

int mINotifyFd,mEpollFd,i;

char inotifyBuf[MAXCOUNT];

char epollBuf[MAXCOUNT];

typedef struct t_name_fd {
	int fd;
	char name[30];

}  T_name_fd;


T_name_fd  t_name_fd[100];
int count_name_fd;





int getfdFromName(char* name)
{
	int i;
	for(i=0; i<MAX_FILES; i++)
	{
		if (!epoll_files[i])
			continue;

		if(0 == strcmp(name,epoll_files[i]))
		{
			return i;

		}
	}

	return -1;
}



int main(int argc, char** argv)
{

	struct epoll_event eventItem;

	struct inotify_event  inotifyEvent;

	struct inotify_event*  curInotifyEvent;
	char name[30];
	int readCount = 0;
	int fd;

	// 1. init inotify &  epoll
	mINotifyFd = inotify_init();
	mEpollFd = epoll_create(10);


	// 2.add inotify watch dir
	inotify_add_watch(mINotifyFd, "/work/test/tmp", IN_DELETE | IN_CREATE);//监听xxx目录下的 delete、create事件

	// 3. add inotify fd to epoll
	eventItem.events = EPOLLIN;
	eventItem.data.fd = mINotifyFd;
	epoll_ctl(mEpollFd, EPOLL_CTL_ADD, mINotifyFd, &eventItem);


	while(1)
	{


		// 4.epoll检测文件的可读变化
		int pollResult = epoll_wait(mEpollFd, mPendingEventItems, EPOLL_COUNT, -1);
		printf("pollResult = %d\n",pollResult);


		for(i=0; i <pollResult; i++)
		{
			// 5. inotify 监听的fd发生变化
			if(mPendingEventItems[i].data.fd == mINotifyFd)
			{

				printf("dir changer------------------\n");

				/* 读取inotify事件，查看是add 文件还是remove文件，
				   add 需要将其添加到epoll中去，
				   remove 需要从epoll中移除
				 */
				readCount  = 0;
				readCount = read(mINotifyFd,inotifyBuf,MAXCOUNT);

				if (readCount <  sizeof(inotifyEvent))
				{
					printf("eorr inofity event\n");
					return -1;
				}

				// cur 指针赋值
				curInotifyEvent = (struct inotify_event*)inotifyBuf;

				while(readCount >= sizeof(inotifyEvent))
				{
					if (curInotifyEvent->len > 0)
					{
						if(curInotifyEvent->mask & IN_CREATE)
						{
							printf("add file :%s\n",curInotifyEvent->name);

							sprintf(name,"/work/test/tmp/%s",curInotifyEvent->name);
							// 打开文文件并将其添加到epoll
							fd=open(name,O_RDWR);
							eventItem.events = EPOLLIN;
							eventItem.data.fd = fd;
							epoll_ctl(mEpollFd, EPOLL_CTL_ADD, fd, &eventItem);

							// 关联name fd
							epoll_files[fd] = name;

							printf("file name ====== :%s\n",name);


						}
						else if(curInotifyEvent->mask & IN_DELETE)
						{
							sprintf(name,"/work/test/tmp/%s",curInotifyEvent->name);

							fd = getfdFromName(name);
							printf("remove file :%s,fd = %d\n",name,fd);

							if (fd >=0)
							{
								epoll_ctl(mEpollFd, EPOLL_CTL_DEL, fd, NULL);
								epoll_files[fd] = NULL;
							}       

						}
					}
					curInotifyEvent --;
					readCount -= sizeof(inotifyEvent);
				}

			}


			//6. 其他原有的fd发生变化
			else
			{
				printf("file changer------------------\n");


				readCount = 0;
				readCount = read(mPendingEventItems[i].data.fd,epollBuf,MAXCOUNT);
				if(readCount > 0)
				{
					epollBuf[readCount] = '\0';
					printf("file can read, fd: %d, countent:%s",mPendingEventItems[i].data.fd,epollBuf);
				}
			}


		}
	}



	return 0;




}


