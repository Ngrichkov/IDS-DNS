#include "HashTable.h" 

#include <pthread.h>
#include <stdio.h>   
#include <stdlib.h>  
#include <string.h>  
#include <ctype.h>   

#define TABLE_SIZE 1024

hashhead* Hash_table[TABLE_SIZE];

int hash(unsigned char *str){
        unsigned long hash = 5381;
        int c;
        while ((c = *str++))
            hash = ((hash << 5) + hash) + c;
        return (int)(hash%TABLE_SIZE);
}

void initHash(){
    for(int i=0; i<TABLE_SIZE; i++){
        Hash_table[i]=NULL;
    }
}

void putClient(int new_id, char* new_topic){
    int h=hash((unsigned char*)new_topic);
    hashhead *step=Hash_table[h];
    hashhead *save=NULL;

    while(step!=NULL){
        if(strcmp(step->topic, new_topic)==0){
            cel *new=malloc(sizeof(cel));

            new->next=step->clients;
            new->id=new_id; 

            step->clients=new;
    
            return;
        }

        save=step;
        step=step->next;
    }

    hashhead *new=malloc(sizeof(hashhead));

    int len=strlen(new_topic)+1;
    new->topic=malloc(len*sizeof(char));

    for(int i=0; i<len-1; i++){
        new->topic[i]=new_topic[i];
    }
    new->topic[len-1]='\0';

    new->clients=malloc(sizeof(cel));
    new->clients->id=new_id;
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

            free(aux_head->topic);

            hashhead *temp_head = aux_head;
            aux_head = aux_head->next;
            free(temp_head);
        }
        Hash_table[i] = NULL;
    }
}