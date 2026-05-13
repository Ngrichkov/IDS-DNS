#ifndef _ST_H
#define _ST_H

#define TABLE_SIZE 1024

#include <pthread.h>

typedef struct Cel{
    struct Cel* next;
    int id;
} cel;

typedef struct Hashhead{
    struct Hashhead* next;
    cel* clients;
    char* topic;
} hashhead;

int hash(unsigned char *str);

void  
initHash();

void  
putClient(int new_id, char* new_topic);

void  
freeHash();

extern hashhead* Hash_table[TABLE_SIZE];

#endif /* _ST_H */