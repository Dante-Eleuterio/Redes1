#include "header.h"
void constroi_buffer(int soquete,unsigned char input[]){
    unsigned char *sendbuff;
    unsigned char dados[64];
    sendbuff= (unsigned char*)malloc(BYTES);
    memset(sendbuff,0,BYTES);
    header *eth = (header *)(sendbuff);
    eth->mi=126;
    eth->tamanho=11;
    eth->sequencia=8;
    eth->tipo=63;
    int total_len=sizeof(header);
    for (int i = total_len; i < BYTES-1; i++)
    {
        sendbuff[i] = input[i-total_len];
    }
        sendbuff[BYTES-1]=111;
    printf("\n%ld\n",sendto(soquete,sendbuff,BYTES,0,NULL,0));

  
}


int main(int argc, char const *argv[])
{
    int send_len= ConexaoRawSocket("lo");
    unsigned char input[64];
    for (int i = 0; i < 64; i++)
    {
        input[i]=0;
    }
    
    if(send_len<0)
    {
        printf("error in sending....sendlen=%d....errno=%d\n",send_len,errno);
        return -1;
    }
    while(1){
        scanf("%s",input);
        constroi_buffer(send_len,input);
    }





  return 0;
}