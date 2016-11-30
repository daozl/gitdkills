#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h> 

#define PORT			8549
#define BUFFSIZE		1024

int main(int argc,char **argv)
{
	int sockfd;
	struct sockaddr_in serv;
	char buff[BUFFSIZE];

	sockfd = socket(AF_INET,SOCK_DGRAM,0);
	if(sockfd < 0){
		perror("socket faild");
		exit(1);
	}

	memset((char *)&serv,0,sizeof(serv));
	
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = inet_addr("172.17.0.18");
	serv.sin_port = htons(PORT);

	recvfrom(sockfd,buff,BUFFSIZE,0,(struct sockaddr *)NULL,(int *)NULL);

		
	puts(buff);

	exit(0);
}
