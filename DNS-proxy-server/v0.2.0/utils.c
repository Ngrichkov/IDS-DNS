#include "utils.h"

#define TABLE_SIZE 1024

/*----------------------------------------------------------------*/

/*Linked List*/

cel* add_cel(cel *head,struct sockaddr_in client, int id){
    cel *new=malloc(sizeof(cel));
    new->next=head;
    new->client=client;
    new->id=id;
    
    return new;
}

cel* rem_cel(cel *head, int id){
    if (!head) return NULL;

    cel *aux=head;
    cel *save=NULL;

    while(aux && aux->id!=id){
        save=aux;
        aux=aux->next;
    }

    if(aux){
        if(save){ 
            save->next=aux->next;   
        }else{
            head=aux->next;
        }
        free(aux);
    }

    return head;
}

/*----------------------------------------------------------------*/

/*Parsers*/

char* conf_parser(char *buffer){
    if(buffer==NULL) return NULL;

    int start=0;
    while(buffer[start]!='=') start+=1;
    
    if(buffer[start+1]=='\0') return NULL;

    int end=start;
    while(buffer[end]!='\n' && buffer[end]!='\0') end+=1;

    int len=end-start-1;
    if (len==0) return NULL;

    char* value=malloc(len+1);
    for(int i=0; i<len; i++){
        value[i]=buffer[start+1+i];
    }

    value[len]='\0';

    return value;
}

char* dns_parser(char* buffer){
    int len_total=0;
    int i=12;
    while(buffer[i]!=0){
        len_total+=(int)buffer[i]+1;
        i+=(int)buffer[i]+1;
    }
    char* hostname=malloc(len_total+1);
    i=12;
    int pos=0;
    while(buffer[i]!=0){
        int len_part=(int)buffer[i];
        for (int j=1; j<=len_part; j++){
            hostname[pos]=buffer[i+j];
            pos++;
        }
        hostname[pos]='.';
        pos++;
        i+=(int)buffer[i]+1;
    }
    hostname[pos-1]=0;
    return hostname;
}

/*----------------------------------------------------------------*/

/*Hashtable*/

hashhead* Hash_table[TABLE_SIZE];

uint32_t hash(uint32_t x) {
    x = ((x >> 16) ^ x) * 0x45d9f3bu;
    x = ((x >> 16) ^ x) * 0x45d9f3bu;
    x = (x >> 16) ^ x;
    return x;
}

void initHash(){
    for(int i=0; i<TABLE_SIZE; i++){
        Hash_table[i]=NULL;
    }
}

void putClient(int id, struct sockaddr_in client){
    int h=hash((uint32_t) id);
    hashhead *step=Hash_table[h];
    hashhead *save=NULL;

    while(step!=NULL){
        if(id==step->id){
            cel *new=malloc(sizeof(cel));

            new->next=step->next;
            new->id=id;

            step->clients=new;
    
            return;
        }

        save=step;
        step=step->next;
    }

    hashhead *new=malloc(sizeof(hashhead));

    new->clients=malloc(sizeof(cel));
    new->clients->id=id;
    new->clients->next=NULL;
    
    new->next=NULL;

    if (save==NULL){
        Hash_table[h]=new;
    } else {
        save->next = new;
    }
}



void freeHash() {
    for (int i = 0; i < TABLE_SIZE; i++) {
        hashhead *aux_head = Hash_table[i];
        while (aux_head != NULL) {
            cel *aux_cel = aux_head->clients;
            while (aux_cel != NULL) {
                cel *temp_aux = aux_cel;
                aux_cel = aux_cel->next;
                free(temp_aux);
            }

            free(aux_head->clients);

            hashhead *temp_head = aux_head;
            aux_head = aux_head->next;
            free(temp_head);
        }
        Hash_table[i] = NULL;
    }
}

/*----------------------------------------------------------------*/