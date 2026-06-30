#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <assert.h>
#include <unistd.h>

#include "utils.h"

#define MAXLINE 4096

void print_client_info(struct sockaddr_in cli_addr){
    char endereco[16];
    printf("Família: %i\n", cli_addr.sin_family);
    printf("Porta: %i\n", ntohs(cli_addr.sin_port));
    printf("IP: %s\n", inet_ntop(AF_INET, &(cli_addr.sin_addr).s_addr, endereco, sizeof(endereco)));
    printf("/------------------------------/\n");
}

//sem_t mutex;

int main(int argc, char** argv){
    cel* head=NULL;

    int listen_fd, cli_len, sockfd;
    struct sockaddr_in serv_addr, cli_addr, name_server_addr, oracle_addr;
    char recv_line[MAXLINE+1];
    char permissao[2];
    
    //sem_init(&mutex, 1, 0);

    ssize_t n;

    if(argc!=2){
        perror("Uso: <endereço IP do oráculo> (ex: 127.0.0.1)\n");
        exit(1);
    }

    if ((listen_fd = socket(AF_INET, SOCK_DGRAM, 0))==-1){
        perror("Erro de socket\n");
        exit(1);
    }

    //informações do cliente
    bzero(&cli_addr, sizeof(cli_addr));
    cli_len = sizeof(cli_addr);
    //servidor DNS proxy
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(8080);
    //oráculo
    bzero(&oracle_addr, sizeof(oracle_addr));
    oracle_addr.sin_family=AF_INET;
    oracle_addr.sin_port = htons(8081);
    inet_pton(AF_INET, argv[1], &oracle_addr.sin_addr.s_addr);
    //servidor DNS verdadeiro
    bzero(&name_server_addr, sizeof(name_server_addr));
    name_server_addr.sin_family=AF_INET;
    name_server_addr.sin_port=htons(53);
    inet_pton(AF_INET, "8.8.8.8", &name_server_addr.sin_addr);

    if (bind(listen_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1){
        perror("Erro de bind.\n");
        exit(1);
    }

    for ( ; ; ){
        n = recvfrom(listen_fd, recv_line, MAXLINE, 0,(struct sockaddr *) &cli_addr, (socklen_t *) &cli_len);
        print_client_info(cli_addr);

        switch((unsigned char)recv_line[2]>>7){
            case 0:{
                int cli_id = ((unsigned char)recv_line[0] << 8) | (unsigned char)recv_line[1];

                head=add_cel(head, cli_addr, cli_id);

                assert((sockfd = socket(AF_INET, SOCK_STREAM, 0)) >= 0);

                assert(connect(sockfd, (struct sockaddr*) &oracle_addr, sizeof(oracle_addr))>=0);

                read(sockfd, permissao, 2);

                close(sockfd);

                permissao[1]=0;

                if(permissao[0]==1){
                    sendto(listen_fd, recv_line, n, 0, (struct sockaddr*) &name_server_addr, sizeof(name_server_addr));
                    printf("Liberado\n");
                }else{
                    printf("Bloqueado\n");
                }
                break;
            }

            case(1):{
                int cli_id = ((unsigned char)recv_line[0] << 8) | (unsigned char)recv_line[1];

                cel *save=NULL;
                for(cel* aux=head; aux; aux=aux->next){
                    if(aux->id==cli_id){
                        sendto(listen_fd, recv_line, n, 0, (struct sockaddr*) &aux->client, sizeof(aux->client));
                        head=rem_cel(head, cli_id);
                        break;
                    }
                    save=aux;
                }
            break;
            }
        }
    }
}