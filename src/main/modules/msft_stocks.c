#include <stdio.h>
#include <stdint.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

#include "msft_stocks.h"

#define MSFT_PORT 117

#ifndef MSFT_IP
	#define MSFT_IP "192.168.1.68"
#endif


const uint32_t buff = 256;

void check_back_with_server(Globals *global){
	char buffer[buff];
	int sockfd, status, len=buff;
	int opt=1;
	struct sockaddr_in servaddr,cli;

	sockfd = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd <= 0){
		fprintf(stderr,"Socket creation failed...\n");
		return ;
	}
	memset(&servaddr,0,sizeof(struct sockaddr_in));
	memset(&buffer,0,sizeof(char)*buff);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(MSFT_PORT);
	if(inet_pton(AF_INET,MSFT_IP,&servaddr.sin_addr)<=0){
		fprintf(stderr,"\nInvalid address / Address not supported\n");
		return ;
	}
	if((status=connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr)))<0){
		fprintf(stderr,"\nConnection failed\n");
		return ;
	}
	send(sockfd,"1",1,0);
	read(sockfd,buffer,buff-1);
	printf("%s\n",buffer);

	close(sockfd);
}

int main(){

	Globals *global = (Globals *) malloc(sizeof(Globals));
	check_back_with_server(global);

	return 0;
}
