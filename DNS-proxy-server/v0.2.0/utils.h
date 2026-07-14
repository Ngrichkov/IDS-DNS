#ifndef _UTILS
#define _UTILS
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdint.h>

#define TABLE_SIZE 1024


/*----------------------------------------------------------------*/
/*Linked List */
struct Cel{
    struct sockaddr_in client;
    int id;
    struct Cel* next;
};
typedef struct Cel cel;

cel* add_cel(cel *head,struct sockaddr_in client, int id);

cel* rem_cel(cel *head, int id);

/*----------------------------------------------------------------*/

/*Parsers*/
char* conf_parser(char *buffer);

char* dns_parser(char *buffer);

/*----------------------------------------------------------------*/

/*Hashtable*/
struct Hashhead{
    struct Hashhead *next;
    char* id;
    cel* clients;
};
typedef struct Hashhead hashhead;

uint32_t hash(uint32_t x);

void initHash();

void putClient(int id, struct sockaddr_in client);

void freeHash();

extern hashhead* Hash_table[TABLE_SIZE];

/*----------------------------------------------------------------*/

#endif 