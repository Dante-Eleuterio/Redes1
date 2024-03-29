#include "BufferFunctions.h"
#include "header.h"


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
    unsigned long mask[BYTES];
    memset(mask,-1,sizeof(unsigned long)*BYTES);
    for (int i = 0; i < BYTES; i++)
        mask[i]=(unsigned long) sendbuff[i];

    usleep(0);
    send(soquete,mask,sizeof(unsigned long)*BYTES,0);
    free(sendbuff);
}

int DesmontaBuffer(unsigned long mask[],unsigned char dados[],int *tipo,int *last_seq,int *seq_rec){
    unsigned char buffer[BYTES];
    for (int i = 0; i < BYTES; i++)
    {
        buffer[i]= (unsigned char) mask[i];
    }
    
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
        return 101;
    }
    for (int i = 0; i < head->tamanho; i++){
        paridade^=data[i];
    }
    if(paridade!=buffer[BYTES-1]){
        fprintf(stderr,"%d  %d  %d  %d\n",head->sequencia, msg_esperada, paridade, buffer[BYTES-1]);
        *tipo=NACK;
        return DEFAULT;
    }
    switch (msg_esperada){
        case 0:
            if(head->sequencia==12 || head->sequencia==13 || head->sequencia==14 || head->sequencia==15){
                return FEITO;
            }
            break;
        case 1:
            if(head->sequencia==13 || head->sequencia==14 || head->sequencia==15 || head->sequencia==0){
                return FEITO;
            }
            break;
        case 2:
            if(head->sequencia==14 || head->sequencia==15 || head->sequencia==0 || head->sequencia==1){
                return FEITO;
            }
            break;
        case 3:
            if(head->sequencia==15 || head->sequencia==0 || head->sequencia==1 || head->sequencia==2){
                return FEITO;
            }
            break;
        default:
            if(head->sequencia<msg_esperada && head->sequencia>=msg_esperada-4){
                return FEITO;
            }
            break;
    }
    if(head->sequencia != msg_esperada){
        *tipo=NACK;
        return DEFAULT;
    }
    *last_seq=head->sequencia;
    *tipo=head->tipo;
    memcpy(dados,data,63);

    if(head->tipo==NACK || head->tipo==ACK){
        return dados[0];
    }
    
    return head->tamanho;
}


void limpa_string(unsigned char input[],int n){
    memset(input,0,n);
}