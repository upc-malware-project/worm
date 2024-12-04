#include <stdio.h>
#include <stdint.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "msft_stocks.h"
#include "globals.h"

#define MSFT_PORT 117

#ifndef MSFT_IP
	#define MSFT_IP "192.168.1.68"
#endif


const uint32_t buff = 256;

void host_msft_server(Globals *global,char* action){
	char buffer[buff];
	int sockfd, comfd,len=buff;
	int opt=1;
	struct sockaddr_in servaddr;
	socklen_t addrlen = sizeof(servaddr);
	struct timeval timeout;
	timeout.tv_sec = 7;
	timeout.tv_usec = 7;

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd <= 0){
		fprintf(stderr,"Socket creation failed...\n");
		return ;
	}
	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR|SO_REUSEPORT,&opt,sizeof(opt))){
		fprintf(stderr,"\nSocket Failed\n");
		return ;
	}
	if(setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout))){
		fprintf(stderr,"\nError: Setting socket timeout\n");
		return ;
	}
	memset(&servaddr,0,sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(MSFT_PORT);

	if(bind(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr))<0){
		fprintf(stderr,"\nBind failed\n");
		return ;
	}
	if(listen(sockfd,9)<0){
		fprintf(stderr,"\nListen failed\n");
		return ;
	}
	if((comfd = accept(sockfd,(struct sockaddr*)&servaddr,&addrlen))<0){
		fprintf(stderr,"\nAcceopt connection failed\n");
		return ;
	}
	read(comfd,buffer,buff-1);
	send(comfd,"1",1,0);


	close(comfd);
	close(sockfd);
}

int main(){
	Globals *global = (Globals*)malloc(sizeof(Globals));
	char action = '0';
	host_msft_server(global,&action);

	return 0;
}
