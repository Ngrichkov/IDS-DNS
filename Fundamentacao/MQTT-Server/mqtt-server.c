/*Algumas constantes que serão utilizadas pelo código.*/
#define MQTT_HEADER_LEN 2
#define MQTT_ACK_LEN    4

#define CONNECT_BYTE  0x10
#define CONNACK_BYTE  0x20

#define SUBACK_BYTE   0x90
#define SUB_BYTE      0X82

#define PUBLISH_BYTE  0x30
/*-----------------------------------------------------*/

#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>

#include "HashTable.h"

#define TABLE_SIZE 1024

#define LISTENQ 1
#define MAXDATASIZE 100
#define MAXLINE 4096

struct connack {
    uint8_t byte_header_flags; 
    uint8_t byte_len;
    uint8_t byte_ack_flags;
    uint8_t byte_ret;
}__attribute__((packed));

struct subsack {
    uint8_t byte_header_flags;
    uint8_t byte_len;
    uint16_t byte_msg_iden;
    uint8_t byte_QoS;
}__attribute__((packed));

int send_connack(int socket){
    struct connack resp = {
        .byte_header_flags = CONNACK_BYTE, 
        .byte_len = 0x02,
        .byte_ack_flags = 0x00,
        .byte_ret = 0x00
        };
    if(send(socket, &resp, sizeof(resp), 0)>0)
        return 1;
    return 0;
}

int send_subsack(int socket, int id){
    struct subsack resp = {
        .byte_header_flags = SUBACK_BYTE,
        .byte_len = 0x03,
        .byte_msg_iden = htons(id),
        .byte_QoS = 0x00
    };
    if(send(socket, &resp, sizeof(resp),0)>0)
        return 1;
    return 0;
}

int send_publish(char *topic, int topic_len, char *msg, int msg_len){
    int hash_number = hash((unsigned char*)topic);
    hashhead* aux_head=Hash_table[hash_number];
    while(aux_head!=NULL && strcmp(aux_head->topic, topic)!=0){
        aux_head=aux_head->next;
    }
    if (aux_head==NULL){
        return 0;   
    }
    
    cel* aux_cel = aux_head->clients;

    unsigned char* pacote = malloc(sizeof(char)*(4+topic_len+msg_len));
    
    pacote[0]=PUBLISH_BYTE;
    pacote[1]=2+topic_len+msg_len;
    pacote[2]=(topic_len>>8) & 0xFF;
    pacote[3]=topic_len & 0xFF;

    memcpy(&pacote[4], topic, topic_len);
    memcpy(&pacote[4+topic_len], msg, msg_len);

    while(aux_cel != NULL) {
        send(aux_cel->id, pacote, 4+topic_len+msg_len, 0);
        aux_cel=aux_cel->next;
    }

    free(pacote);
    return 1;
}

void* operacao(void* args){
    /**** PROCESSO FILHO ****/
    printf("[Uma conexão aberta]\n");
   
    int connfd=*(int*)args;
    free(args);

    unsigned char recvline[MAXLINE + 1];
    ssize_t n;

    while ((n = read(connfd, recvline, MAXLINE)) > 0) {
        recvline[n]=0;

        printf("/-----------------------------------/\n");
        switch (recvline[0]) {
        case CONNECT_BYTE: //PEDIDO CONNECT
            printf("Pedido CONNECT recebido!\n");
            send_connack(connfd);
            printf("Connack enviado!\n");
            break;

        case SUB_BYTE: //PEDIDO SUBSCRIBE
            printf("Pedido SUBSCRIBE recebido!\n");

            uint16_t id = (recvline[2] << 8) | recvline[3];

            uint16_t topic_len=(recvline[4] << 8) | recvline[5];

            char topic[256];

            memcpy(topic, &recvline[6], topic_len);

            topic[topic_len] = '\0';

            sem_wait(&mutex);
            putClient(connfd, topic);
            sem_post(&mutex);

            send_subsack(connfd, id);                    
            
            printf("Subsack enviado\n");
            break;

        case PUBLISH_BYTE:{ //PEDIDO PUBLISH
            printf("Pedido publish recebido!\n");

            int topic_len=recvline[2]<<8 | recvline[3];
            int msg_len=recvline[1]-2-topic_len;

            char topic[256];
            memcpy(topic, &recvline[4], topic_len);
            topic[topic_len] = '\0';
            
            char* msg=(char*)&recvline[4+topic_len];

            sem_wait(&mutex);
            send_publish(topic, topic_len, msg, msg_len);
            sem_post(&mutex);
            break;
        }
        default:
            printf("\nERRO: O cliente enviou uma mensagem de tipo não aceita\n");
            break;
        }
    }
    printf("[Uma conexão fechada]\n");
    close(connfd);
    return NULL;
}

sem_t mutex;

int main (int argc, char **argv) {
    initHash();

    pthread_t thread;

    sem_init(&mutex, 0, 1);

    int listenfd, connfd;
    
    struct sockaddr_in servaddr;
    
    pid_t childpid;
    
    unsigned char recvline[MAXLINE + 1];
    
    ssize_t n;
   
    if (argc != 2) {
        fprintf(stderr,"Uso: %s <Porta>\n",argv[0]);
        fprintf(stderr,"Vai rodar um servidor de echo na porta <Porta> TCP\n");
        exit(1);
    }

    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket :(\n");
        exit(2);
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(atoi(argv[1]));
    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind :(\n");
        exit(3);
    }

    if (listen(listenfd, LISTENQ) == -1) {
        perror("listen :(\n");
        exit(4);
    }

    printf("[Servidor no ar. Aguardando conexões na porta %s]\n",argv[1]);
    printf("[Para finalizar, pressione CTRL+c ou rode um kill ou killall]\n");
   
	for (;;) {
        if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
            perror("accept :(\n");
            exit(5);
        }

        int* arg_fd=malloc(sizeof(int));
        *arg_fd=connfd;

        pthread_create(&thread, NULL, operacao, arg_fd);
        pthread_detach(thread);
    }
    exit(0);
}
