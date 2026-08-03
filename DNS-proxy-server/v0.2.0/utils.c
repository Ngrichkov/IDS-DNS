#include "utils.h"

#define TABLE_SIZE 1024

/*----------------------------------------------------------------*/

/*Linked List*/

cel* add_cel(cel *head,struct sockaddr_in client, char* id){
    cel *new=malloc(sizeof(cel));
    new->next=head;
    new->client=client;
    new->id=strdup(id);
    
    return new;
}

cel* rem_cel(cel *head, char* id){
    if (!head) return NULL;

    cel *aux=head;
    cel *save=NULL;

    while(aux && strcmp(aux->id, id)!=0){
        save=aux;
        aux=aux->next;
    }

    if(aux){
        if(save){ 
            save->next=aux->next;   
        }else{
            head=aux->next;
        }
        free(aux->id);
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

uint32_t hash(char *str) {
    uint32_t hash=5381;
    int c;
    while((c=*str++)){
        hash=((hash<<5)+hash)+c;
    }
    return hash % TABLE_SIZE;
}

void initHash(){
    for(int i=0; i<TABLE_SIZE; i++)
        Hash_table[i]=NULL;
}

void putClient(char* id, struct sockaddr_in client){
    int h=hash(id);
    hashhead *step=Hash_table[h];

    while(step!=NULL){
        if(strcmp(id, step->id)==0){
            cel *new=malloc(sizeof(cel));
            new->client=client;
            new->id=strdup(id);
            new->next=step->clients;
            step->clients=new;
            return;
        }
        step=step->next;
    }

    hashhead *new=malloc(sizeof(hashhead));
    new->clients=malloc(sizeof(cel));
    new->id=strdup(id);
    new->clients->next=NULL;
    new->clients->id=strdup(id);
    new->clients->client=client;
    
    new->next=Hash_table[h];
    Hash_table[h]=new;
}

int getClient(char* id, struct sockaddr_in *client){
    int h=hash(id);
    hashhead *step=Hash_table[h];
    hashhead *save=NULL;

    while(step!=NULL){
        if(strcmp(id, step->id)==0){
            if(step->clients!=NULL){
                *client=step->clients->client;
                
                cel *temp=step->clients;
                step->clients=step->clients->next;
                
                free(temp->id);
                free(temp);
                
                if (step->clients==NULL) {
                    if (save==NULL) Hash_table[h]=step->next;
                    else save->next=step->next;
                    free(step->id); 
                    free(step);
                }
                return 1;
            }
            return 0; 
        }
        save=step;
        step=step->next;
    }
    return 0;
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