#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

#define MAXLINE		1024
#define IPADDRESS	"172.17.0.130"    //server端ip
#define SERV_PORT	8787		   //server端口号

#define max(a,b)	((a) > (b) ? (a) : (b))  

static void handle_recv_msg(int sockfd,char *buf)     //处理接收的信息
{
	printf("client recv msg is:%s\n",buf);
	sleep(5);
	write(sockfd,buf,strlen(buf)+1);
}
static void handle_connection(int sockfd)           //处理已经建立连接的sockfd
{
	char sendline[MAXLINE],recvline[MAXLINE];
	int maxfdp,stdineof;
	fd_set readfds;                                //定义文件描述符集合  
	int n;
	struct timeval tv;								//定义超时时间					
	int retval = 0;

	while(1){
		
		FD_ZERO(&readfds);                       //清空描述符集合
		FD_SET(sockfd,&readfds);				 //将sockfd 添加到readfds
		maxfdp = sockfd;							

		tv.tv_sec = 5;
		tv.tv_usec = 0;

		retval = select(maxfdp+1,&readfds,NULL,NULL,&tv);         

		if(retval == -1){
			perror("select()");
			exit(1);
		}
		if(retval == 0){
			printf("client timeout.\n");
			continue;
		}
		if(FD_ISSET(sockfd,&readfds)){
			n = read(sockfd,recvline,MAXLINE);
			if(n <= 0){
				fprintf(stderr,"client: server is closed.\n"); //总之，当对方关闭，一执行recv()或read()，马上回返回-1，此时全局变量errno的值是115
				close(sockfd);                                 //当有东西可读时，一定要检查recv()或read()的返回值，返回-1时要作出关断本地Socket的处理
				FD_CLR(sockfd,&readfds);                       //，否则select()会一直认为有东西读，   
				return;
			}
			handle_recv_msg(sockfd,recvline);                
		}
	}
}
int main(int argc,char **argv)
{
	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = socket(AF_INET,SOCK_STREAM,0);

	memset((char *)&servaddr,0,sizeof(servaddr));            //将servaddr清零

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	inet_pton(AF_INET,IPADDRESS,&servaddr.sin_addr);

	int retval = 0;
	retval = connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
	if(retval < 0){
		fprintf(stderr,"connect fail.error:%s\n",strerror(errno));
		exit(1);
	}

	printf("client send to server.\n");
	write(sockfd,"hello server",32);

	handle_connection(sockfd);

	exit(0);
}
