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

int main(){
    char buffer[MAXLINE+1];
    
    FILE *fptr;

    fptr=fopen("conf.txt", "r");
    assert(fptr!=NULL);

    fgets(buffer, MAXLINE+1, fptr);

    char* DNS_server_address=conf_parser(buffer);
    assert(DNS_server_address!=NULL);

    fgets(buffer, MAXLINE+1, fptr);

    char* proxy_server_port=conf_parser(buffer);
    assert(proxy_server_port!=NULL);
    
    fgets(buffer, MAXLINE+1, fptr);

    char* oracle_server_port=conf_parser(buffer);
    assert(oracle_server_port!=NULL);

    fgets(buffer, MAXLINE+1, fptr);

    char* oracle_server_address=conf_parser(buffer);
    assert(oracle_server_address!=NULL);

    cel* head=NULL;

    int listen_fd, cli_len, sockfd;
    struct sockaddr_in serv_addr, cli_addr, name_server_addr, oracle_addr;
    
    char permission[2];

    ssize_t n;

    if ((listen_fd = socket(AF_INET, SOCK_DGRAM, 0))==-1){
        perror("sockerr\n");
        exit(1);
    }

    bzero(&cli_addr, sizeof(cli_addr));
    cli_len = sizeof(cli_addr);

    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(strtol(proxy_server_port, NULL, 10));
    free(proxy_server_port);

    bzero(&oracle_addr, sizeof(oracle_addr));
    oracle_addr.sin_family=AF_INET;
    oracle_addr.sin_port = htons(strtol(oracle_server_port, NULL, 10));
    inet_pton(AF_INET, oracle_server_address, &oracle_addr.sin_addr.s_addr);
    free(oracle_server_address);
    free(oracle_server_port);

    bzero(&name_server_addr, sizeof(name_server_addr));
    name_server_addr.sin_family=AF_INET;
    name_server_addr.sin_port=htons(53);
    inet_pton(AF_INET, DNS_server_address, &name_server_addr.sin_addr);
    free(DNS_server_address);

    if (bind(listen_fd, (struct sockaddr*) &serv_addr, sizeof(serv_addr))==-1){
        perror("binderr.\n");
        exit(1);
    }

    initHash();

    for ( ; ; ){
        n = recvfrom(listen_fd, buffer, MAXLINE, 0,(struct sockaddr *) &cli_addr, (socklen_t *) &cli_len);

        switch((unsigned char)buffer[2]>>7){
            case 0:{
                printf("Query recebida;\n");
                int cli_id = ((unsigned char)buffer[0] << 8) | (unsigned char)buffer[1];
                char* hostname=dns_parser(buffer);
                char hash_key[MAXLINE];
                snprintf(hash_key, MAXLINE, "%d%s", cli_id, hostname);

                putClient(hash_key, cli_addr);

                assert((sockfd = socket(AF_INET, SOCK_STREAM, 0))>=0);
                assert(connect(sockfd, (struct sockaddr*) &oracle_addr, sizeof(oracle_addr))>=0);
                read(sockfd, permission, 2);
                close(sockfd);
                
                permission[1]=0;
                if(permission[0]==1){
                    sendto(listen_fd, buffer, n, 0, (struct sockaddr*) &name_server_addr, sizeof(name_server_addr));
                    printf("\tQuery premitida;\n");
                }
                free(hostname);
                break;
            }

            case 1:{
                printf("Resposta recebida;\n");
                int cli_id = ((unsigned char)buffer[0] << 8) | (unsigned char)buffer[1];
                char* hostname = dns_parser(buffer);
                char hash_key[MAXLINE];
                snprintf(hash_key, MAXLINE, "%d%s", cli_id, hostname);
                
                struct sockaddr_in target_client;
                if (getClient(hash_key, &target_client)) {
                    sendto(listen_fd, buffer, n, 0, (struct sockaddr*) &target_client, sizeof(target_client));
                }
                
                free(hostname);
                break;
            }
        }
    }
}