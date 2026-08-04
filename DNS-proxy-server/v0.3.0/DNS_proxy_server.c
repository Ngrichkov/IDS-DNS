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
#include <pthread.h>
#include <time.h>
#include <math.h>

#include "utils.h"

#define MAXLINE 4096

void* sampling(void* arg){
    struct sockaddr_in* dns_server=(struct sockaddr_in*) arg;
    int samp_socket = socket(AF_INET, SOCK_DGRAM, 0);

    char probe[]={"\261?\001 \000\001\000\000\000\000\000\001\006google\003com\000\000\001\000\001\000\000)\004\320\000\000\000\000\000\f\000\n\000\b\177\237\3560\241E{\233"};
    char buffer[MAXLINE];

    while(1){
        double times[100];
        double sum=0.0;

        for(int i = 0; i < 100; i++) {
            struct timespec start, end;
            
            clock_gettime(1, &start);
            sendto(samp_socket, probe, sizeof(probe), 0, (struct sockaddr*)dns_server, sizeof(*dns_server));
            recvfrom(samp_socket, buffer, 512, 0, NULL, NULL);
            clock_gettime(1, &end);

            double elapsed=(end.tv_sec-start.tv_sec)+(end.tv_nsec-start.tv_nsec)/1e9;
            times[i]=elapsed;
            sum += elapsed;
        }

        double mean=sum/100;
        double var=0.0;
        for(int i=0; i<100; i++){
            var+=pow((times[i]-mean), 2);
        }

        double std_dev=sqrt(var/100.0);

        pthread_mutex_lock(&ttl_mutex);
        ttl=1.65*std_dev+mean;
        pthread_mutex_unlock(&ttl_mutex);

        sleep(300);
    }
}

void* supervisor(void* arg) {
    while(1) {
        sleep(10);

        pthread_mutex_lock(&ttl_mutex);
        double current_ttl = ttl;
        pthread_mutex_unlock(&ttl_mutex);

        struct timespec now;
        clock_gettime(1, &now);

        pthread_mutex_lock(&hash_mutex);
        for(int i = 0; i < TABLE_SIZE; i++) {
            hashhead* h_step=Hash_table[i];
            while(h_step!=NULL){
                cel* c_save=NULL;
                cel* c_step=h_step->clients;
                while(c_step!=NULL){
                    if((now.tv_sec - c_step->time.tv_sec) > current_ttl){
                            cel* trash=c_step;
                            c_step=c_step->next;
                            if(c_save==NULL){
                                h_step->clients=c_step;
                            }else{
                                c_save->next=c_step;
                            }
                            free(trash->id);
                            free(trash);
                    }else{
                        c_save=c_step;
                        c_step=c_step->next;
                    }
                }
                h_step=h_step->next;
            }
        }
        pthread_mutex_unlock(&hash_mutex);
    }
    
    return NULL;
}

double ttl=5.0;
pthread_mutex_t hash_mutex=PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ttl_mutex=PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char** argv){
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

    pthread_t thread_sampling;
    pthread_t thread_supervisor;

    pthread_create(&thread_sampling, NULL, sampling, (void*)&name_server_addr);
    pthread_create(&thread_supervisor, NULL, supervisor, NULL);

    for ( ; ; ){
        n = recvfrom(listen_fd, buffer, MAXLINE, 0,(struct sockaddr *) &cli_addr, (socklen_t *) &cli_len);

        switch((unsigned char)buffer[2]>>7){
            case 0:{
                int cli_id = ((unsigned char)buffer[0] << 8) | (unsigned char)buffer[1];
                char* hostname=dns_parser(buffer);
                char hash_key[MAXLINE];
                snprintf(hash_key, MAXLINE, "%d%s", cli_id, hostname);

                pthread_mutex_lock(&hash_mutex);
                putClient(hash_key, cli_addr);
                pthread_mutex_unlock(&hash_mutex);

                assert((sockfd = socket(AF_INET, SOCK_STREAM, 0))>=0);
                assert(connect(sockfd, (struct sockaddr*) &oracle_addr, sizeof(oracle_addr))>=0);
                read(sockfd, permission, 2);
                close(sockfd);
                
                permission[1]=0;
                if(permission[0]==1){
                    sendto(listen_fd, buffer, n, 0, (struct sockaddr*) &name_server_addr, sizeof(name_server_addr));
                }
                free(hostname);
                break;
            }

            case 1:{
                int cli_id = ((unsigned char)buffer[0] << 8) | (unsigned char)buffer[1];
                char* hostname = dns_parser(buffer);
                char hash_key[MAXLINE];
                snprintf(hash_key, MAXLINE, "%d%s", cli_id, hostname);
                
                struct sockaddr_in target_client;
                pthread_mutex_lock(&hash_mutex);
                int result=getClient(hash_key, &target_client);
                pthread_mutex_unlock(&hash_mutex);
                if (result==1){
                    sendto(listen_fd, buffer, n, 0, (struct sockaddr*) &target_client, sizeof(target_client));
                }
                
                free(hostname);
                break;
            }
        }
    }
}
