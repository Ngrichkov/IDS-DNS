#include <stdio.h>
#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 100 

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

int main(int argc) {
  int sockfd;
  ssize_t n;
  pthread_t thread_id;
  char sendline[MAXLINE];
  struct sockaddr_in servaddr;
  
  if (argc!=2){
    fprintf(stderr, "uso: %s <Endereço IPv4>\n", argv[0]);
    exit(1);
  }

  if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    fprintf(stderr, "socket() failed\n");
    exit(1);
  }

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family=AF_INET;
  servaddr.sin_port=htons(8080);

  if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
    fprintf(stderr,"erro do inet_pton para %s :(\n", argv[1]);
    exit(1);
  }

  if (pthread_create(&thread_id, NULL, listening, &sockfd) != 0) {
    fprintf(stderr, "Erro ao criar a thread :(\n");
    exit(1);
  }

  if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
    fprintf(stderr,"erro do connect :(\n");
    exit(1);
  }

  bzero(sendline, MAXLINE);
  while((fgets(sendline, MAXLINE, stdin)) != NULL) {
    if (write(sockfd, sendline, strlen(sendline)) < 0) {
      printf("Erro ao enviar mensagem :(\n");
      break;
    }

    if (strncmp(sendline, "Adeus", 5) == 0) {
      printf("Encerrando o chat...\n");
      break;
    }

    bzero(sendline, MAXLINE);
  }

  close(sockfd);
  exit(0);
}
