#include <time.h>
#include <sys/socket.h>
#include <strings.h>
#include <string.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 100
#define LISTENQ 1

void *listening(void *arg) {
    int socket = *((int *)arg);
    char recvline[MAXLINE + 1];
    ssize_t n;

    bzero(recvline, MAXLINE);
    while ((n=read(socket, recvline, MAXLINE)) > 0) {
        recvline[n] = '\0';
        printf("[Outro]: %s", recvline);
        bzero(recvline, MAXLINE);
    }

    if (n == 0) {
        printf("\nO outro lado encerrou a conexão.\n");
    } else {
        perror("\nErro ao ler do socket");
    }
    close(socket);
    exit(0);
}

int main(int argc, char **argv) {
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    pthread_t thread_id;
    char sendline[MAXLINE];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        fprintf(stderr,"erro do socket :(\n");
        exit(1);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(8080);

    if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 ) {
        fprintf(stderr,"erro do bind :(\n");
        exit(1);
    }

    if (listen(listenfd, LISTENQ) < 0) {
        fprintf(stderr,"erro do listen :(\n");
        exit(1);
    }

    if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL))< 0) {
        fprintf(stderr,"erro do accept :(\n");
        exit(1);
    }

    if (pthread_create(&thread_id, NULL, listening, &connfd) != 0) {
        fprintf(stderr, "Erro ao criar a thread :(\n");
        exit(1);
    }

    snprintf(sendline, sizeof(sendline), "Conectados!\n\n------------------------------------------------\n\n");
    write(connfd, sendline, strlen(sendline));
    printf("Conectados!\nDigite <Adeus> para sair\n\n---------------------------------------------------------\n\n");

    bzero(sendline, MAXLINE);
    while (fgets(sendline, MAXLINE, stdin) != NULL) {
        if (write(connfd, sendline, strlen(sendline)) < 0) {
            printf("Erro ao enviar mensagem :(\n");
            break;
        }

        if ((strncmp(sendline, "Adeus", 5))==0) {
            printf("Encerrando o chat...\n");
            break;
        }

        bzero(sendline, MAXLINE);
    }
    close(connfd);
    exit(0);
}