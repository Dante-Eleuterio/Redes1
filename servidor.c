#include "header.h"
#include "BufferFunctions.h"

int last_seq;
int sequencia_recebida;
int sequencia;


void falha(int soquete,unsigned char data[]){
  limpa_string(data,63);
  sequencia++;
  if(last_seq==15)
    data[0]=0;
  else
    data[0]=last_seq+1;
  constroi_buffer(soquete,sequencia,data,NACK);
}

void trata_tipo(int tipo,unsigned char data[],int soquete){
  switch (tipo)
  {
  case LS:
      if(data[0]==0)
        system("ls > dados.txt");
      if(data[0]==1)
        system("ls -a > dados.txt");
      if(data[0]==2)
        system("ls -l > dados.txt");
      if(data[0]==3)
        system("ls -a -l > dados.txt");
    system("cat dados.txt");
    break;
  case CD:
    if(sequencia_recebida==last_seq){
      limpa_string(data,63);
      sequencia++;
      constroi_buffer(soquete,sequencia,data,OK);
    }
    else{
      chdir(data);
      limpa_string(data,63);
      sequencia++;
      constroi_buffer(soquete,sequencia,data,OK);
    }
    if(sequencia==16)
      sequencia=0;
    break;
  case MKDIR:
    mkdir(data,0700);
    break;
  case GET:

    break;
  case PUT:

    break;
  case DESCRITOR:

    break;
  case DADOS:

    break;
  case NACK:

    break;
  case ACK:

    break;
  default:
    break;
  }
}

int main(){
  int buflen;
  int sock_r;
  int tipo=-1;
  sock_r = ConexaoRawSocket("enp7s0f0");
  unsigned char *buffer = (unsigned char *) malloc(BYTES); //to receive data
  memset(buffer,0,BYTES);
  unsigned char data[63];
  limpa_string(buffer,67);
  limpa_string(data,63);
  sequencia=-1;
  last_seq=10;
  sequencia_recebida=10;
  while(1)
  {  
    limpa_string(buffer,67);
    limpa_string(data,63);
    buflen=recvfrom(sock_r,buffer,BYTES,0,NULL,0);
    if(buflen<0){
      printf("error in reading recvfrom function\n");
      return -1;
    }
  if(buffer[0]==126){
    if(DesmontaBuffer(buffer,data,&tipo,&last_seq,&sequencia_recebida))
      trata_tipo(tipo,data,sock_r);
    else
      falha(sock_r,data);
  }
  }
  return 0;
}