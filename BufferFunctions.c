#include "BufferFunctions.h"
#include "header.h"

void imprime_buffer(header *head){
      fprintf(stderr,"HEADER");
      fprintf(stderr,"\t|mi :%d\n ",head->mi);
      fprintf(stderr,"\t|tamanho :%d\n ",head->tamanho);
      fprintf(stderr,"\t|sequencia :%d\n ",head->sequencia);
      fprintf(stderr,"\t|tipo :%d\n",head->tipo);
      fprintf(stderr,"\t|dados: ");
      for (int i = 0; i < head->tamanho; i++) {
        unsigned char d = (((unsigned char*)(head)) + sizeof(header))[i];
        if (d < 0x20) {
            fprintf(stderr,"[%d]", d);
        } else {
        fprintf(stderr,"'%c'", d);
        }
      }
      fprintf(stderr,"\n");
}
void constroi_buffer(int soquete,int sequencia,unsigned char input[],int tipo,int tamanho){
    unsigned char *sendbuff;
    sendbuff= (unsigned char*)malloc(BYTES);
    memset(sendbuff,0,BYTES);
    header *head = (header *)(sendbuff);
    head->mi=126;
    head->tamanho=tamanho;
    head->sequencia=sequencia;
    head->tipo=tipo ;
    int header_len=sizeof(header);
    int paridade=0;
    for (int i = header_len; i < tamanho+header_len; i++)
    {
        sendbuff[i] = input[i-header_len];
        paridade^=sendbuff[i];
    }
    sendbuff[BYTES-1]=paridade;
    printf("enviando\n");
    imprime_buffer(head);
    sendto(soquete,sendbuff,BYTES,0,NULL,0);
    free(sendbuff);
}

int DesmontaBuffer(unsigned char buffer[],unsigned char dados[],int *tipo,int *last_seq,int *seq_rec){
    *seq_rec = DEFAULT;
    int msg_esperada=0;
    if(*last_seq!=15){
        msg_esperada=*last_seq+1;
    }
    header *head = (header *)(buffer);
    int paridade=0;
    unsigned char *data = (buffer + sizeof(header));
    if(head->sequencia==*last_seq){
        *tipo=head->tipo;
        *seq_rec=head->sequencia;
        return head->tamanho;
    }
    for (int i = 0; i < head->tamanho; i++){
        paridade^=data[i];
    }
    switch (msg_esperada){
        case 0:
            if(head->sequencia>=13 && head->sequencia<=15)
                return FEITO;
            break;
        case 1:
            if(head->sequencia==14 || head->sequencia==15 || head->sequencia==0)
                return FEITO;
            break;
        case 2:
            if(head->sequencia==15 || head->sequencia==0 || head->sequencia==1)
                return FEITO;
            break;
        default:
            if(head->sequencia<msg_esperada && head->sequencia>=msg_esperada-3)
                return FEITO;
            break;
    }
    if(head->sequencia!=msg_esperada || paridade!=buffer[BYTES-1]){
        fprintf(stderr,"%d  %d  %d  %d\n",head->sequencia, msg_esperada, paridade, buffer[BYTES-1]);
        *tipo=NACK;
        return DEFAULT;
    }
    *last_seq=head->sequencia;
    *tipo=head->tipo;
    memcpy(dados,data,63);

    if(head->tipo==NACK || head->tipo==ACK){
        printf("recebendo\n");
        imprime_buffer(head);
        return dados[0];
    }
    
    return head->tamanho;
}

// void windows_send(char *buffer, int size){


// }

// void file_reader(char *arquivo, char *buffer, int size){


// }

void limpa_string(unsigned char input[],int n){
    memset(input,0,n);
}