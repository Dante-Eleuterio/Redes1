#include "BufferFunctions.h"
#include "header.h"

void imprime_buffer(header *head){
      printf("HEADER");
      printf("\t|mi :%d\n ",head->mi);
      printf("\t|tamanho :%d\n ",head->tamanho);
      printf("\t|sequencia :%d\n ",head->sequencia);
      printf("\t|tipo :%d\n",head->tipo);
}
void constroi_buffer(int soquete,int sequencia,unsigned char input[],int tipo){
    unsigned char *sendbuff;
    sendbuff= (unsigned char*)malloc(BYTES);
    memset(sendbuff,0,BYTES);
    header *head = (header *)(sendbuff);
    head->mi=126;
    head->tamanho=strlen(input);
    head->sequencia=sequencia;
    head->tipo=tipo;
    int header_len=sizeof(header);
    int paridade=0;
    for (int i = header_len; i < BYTES-1; i++)
    {
        sendbuff[i] = input[i-header_len];
        paridade^=sendbuff[i];
    }
    sendbuff[BYTES-1]=paridade;
    imprime_buffer(head);
    sendto(soquete,sendbuff,BYTES,0,NULL,0);
    free(sendbuff);
}

int DesmontaBuffer(unsigned char buffer[],unsigned char dados[],int *tipo,int *last_seq,int *seq_rec){
    header *head = (header *)(buffer);
    int paridade=0;
    unsigned char *data = (buffer + sizeof(header));
    if(head->sequencia==*last_seq){
        *tipo=head->tipo;
        *seq_rec=head->sequencia;
        return 1;
    }
    for (int i = 0; i < head->tamanho; i++){
        paridade^=data[i];
    }
    if(head->sequencia!=*last_seq+1 || paridade!=buffer[BYTES-1]){
        *tipo=NACK;
        return 0;
    }
    *last_seq=head->sequencia;
    *tipo=head->tipo;
    strncpy(dados,data,63);
    imprime_buffer(head);
    return 1;
}


void limpa_string(unsigned char input[],int n){
    memset(input,0,n);
}