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

#define MSFT_PORT 117

#ifndef MSFT_IP
	#define MSFT_IP "192.168.1.68"
#endif


const uint32_t buff = 256;

void check_back_with_server(Globals *global){
	char buffer[buff];
	int sockfd, connfd,len=buff;
	struct sockaddr_in servaddr,cli;

	sockfs = socket(AF_INET,SOCK_STREAM,0);
	if(sockfd == -1){
		fprintf(stderr,"Socket creation failed...\n");
		return 0;
	}
	memset(&servaddr,0,sizeof(struct sockaddr_in));

}

int main(){


	return 0;
}
