#include "header.h"
#include "BufferFunctions.h"

//Flags de controle de sequência
int last_seq;
int aux;
int sequencia;
int tamanho;
//Trata erro de recebimento e envia
void falha(int soquete,unsigned char data[]){
  limpa_string(data,63);
  sequencia++;
  if(last_seq==15)   //Confere se chegou no final da sequencia
    data[0]=0;      
  else
    data[0]=last_seq+1;
  constroi_buffer(soquete,sequencia,data,NACK,1);
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
    system("rm -f dados.txt");
    break;
  case CD:
    if(aux==last_seq){
      limpa_string(data,63);
      sequencia++;
      constroi_buffer(soquete,sequencia,data,OK,0);
    }
    else{
      sequencia++;
      data[tamanho] = '\0';
      if(!chdir(data)){
        // TO DO: Arrumar
        constroi_buffer(soquete,sequencia,data,OK,0);
      }
      else{
        switch (errno){
        case EACCES:
        case EFAULT:
          data[0]='B';
          break;
        case ENOTDIR:
        case ENOENT:
          data[0]='A';
          break;
        default:
          data[0]='Z';
          break;
        }
        constroi_buffer(soquete,sequencia,data,ERRO,1);
      }
    }
    if(sequencia==16)
      sequencia=0;
    break;
  case MKDIR:
      if(aux==last_seq){
      limpa_string(data,63);
      sequencia++;
      constroi_buffer(soquete,sequencia,data,OK,0);
    }
    else{
      sequencia++;
      data[tamanho] = '\0';
      if(!mkdir(data,0700)){
        // TO DO: Arrumar
        constroi_buffer(soquete,sequencia,data,OK,0);
      }
      else{
        switch (errno){
        case EACCES:
        case EFAULT:
          data[0]='B';
          break;
        case EEXIST:
          data[0]='C';
          break;
        case ENOSPC:
          data[0]='E';
          break; 
        default:
          data[0]='Z';
          break;
        }
        constroi_buffer(soquete,sequencia,data,ERRO,1);
      }
    }
    if(sequencia==16)
      sequencia=0;
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
  int tipo=DEFAULT;
  sock_r = ConexaoRawSocket("enp7s0f0");
  unsigned char *buffer = (unsigned char *) malloc(BYTES); //to receive data
  memset(buffer,0,BYTES);
  unsigned char data[63];
  limpa_string(buffer,BYTES);
  limpa_string(data,63);
  sequencia=-1;
  last_seq=15;
  while(1)
  {  
    limpa_string(buffer,BYTES);
    limpa_string(data,63);
    buflen=recvfrom(sock_r,buffer,BYTES,0,NULL,0);
    if(buflen<0){
      printf("error in reading recvfrom function\n");
      return -1;
    }
    if(buffer[0]==126){
      tamanho=DesmontaBuffer(buffer,data,&tipo,&last_seq,&aux);
      if(tamanho!=DEFAULT)
        trata_tipo(tipo,data,sock_r);
      else
        falha(sock_r,data);
    }
  }
  return 0;
}