#include "utils.h"

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