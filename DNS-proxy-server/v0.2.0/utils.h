#ifndef _UTILS
#define _UTILS
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define TABLE_SIZE 1024


/*----------------------------------------------------------------*/
/*Linked List */
struct Cel{
    struct sockaddr_in client;
    char* id;
    struct Cel* next;
};
typedef struct Cel cel;

cel* add_cel(cel *head,struct sockaddr_in client, char* id);

cel* rem_cel(cel *head, char* id);

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

uint32_t hash(char *str);

void initHash();

void putClient(char* id, struct sockaddr_in client);

int getClient(char* id, struct sockaddr_in *client);

void freeHash();

extern hashhead* Hash_table[TABLE_SIZE];

/*----------------------------------------------------------------*/

#endif 