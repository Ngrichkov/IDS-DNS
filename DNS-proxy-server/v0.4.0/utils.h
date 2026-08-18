#ifndef _UTILS
#define _UTILS
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <math.h>

#define TABLE_SIZE 1024

/*----------------------------------------------------------------*/
/*Threads*/
extern double ttl;
extern pthread_mutex_t hash_mutex;
extern pthread_mutex_t ttl_mutex;
extern pthread_mutex_t queue_mutex;

extern pthread_cond_t isEmpty;

/*----------------------------------------------------------------*/
/*Parsers*/
char* conf_parser(char *buffer);

char* dns_parser(char *buffer);

/*----------------------------------------------------------------*/
/*Hashtable*/
struct Cel{
    struct sockaddr_in client;
    char* id;
    struct Cel* next;
    struct timespec time;
};
typedef struct Cel hash_cel;

hash_cel* add_cel(hash_cel *head,struct sockaddr_in client, char* id);

struct Hashhead{
    struct Hashhead *next;
    char* id;
    hash_cel* clients;
};
typedef struct Hashhead hashhead;

uint32_t hash(char *str);

void initHash();

void putClient(char* id, struct sockaddr_in client);

int getClient(char* id, struct sockaddr_in *client);

void freeHash();

extern hashhead* Hash_table[TABLE_SIZE];

/*----------------------------------------------------------------*/
/*Queue*/

typedef struct Queue{
    struct Queue* next;
    struct Queue* prev;
    char* hostname;
    char* buffer;
    int n;
}queue;

extern queue* Head;

void enqueue(char* hostname, char* buffer, ssize_t n, queue* H);

queue* dequeue(queue* H);

/*----------------------------------------------------------------*/

#endif 