#ifndef _UTILS
#define _UTILS
#include <stdio.h>
#include <netinet/in.h>
#include <stdlib.h>

struct Cel{
    int id;
    struct sockaddr_in client;
    struct Cel* next;
};

typedef struct Cel cel;

cel* add_cel(cel *head,struct sockaddr_in client, int id);

cel* rem_cel(cel *head, int id);

#endif 