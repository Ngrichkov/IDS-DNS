#include <netinet/in.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 4096

void* send_random(void* args){
    int* arg_ptr=(int *)args;
    int connfd=*arg_ptr;
    free(arg_ptr);

    char packet[2];

    srand(time(NULL));

    packet[0]=rand()%2;
    packet[1]='\0';

    send(connfd, packet, 2, 0);

    close(connfd);
}

void main(){
    struct sockaddr_in servaddr;
    int listenfd, connfd;
    ssize_t n;
    pthread_t thread;

    assert((listenfd=socket(AF_INET, SOCK_STREAM, 0)) != -1);

    bzero(&servaddr, sizeof(struct sockaddr_in));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(8081);
    servaddr.sin_addr.s_addr=htonl(INADDR_ANY);

    assert((connfd=bind(listenfd, (struct sockaddr*) &servaddr, sizeof(servaddr))) != -1);

    assert((listen(listenfd, 1)) != -1);

    for( ; ; ){
        assert((connfd = accept(listenfd, NULL, NULL)) != -1);
        
        int* arg_fd=malloc(sizeof(int));
        *arg_fd=connfd;

        pthread_create(&thread, NULL, send_random, arg_fd);
        pthread_detach(thread);
    }
}