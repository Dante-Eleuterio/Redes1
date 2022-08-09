#include "header.h"
#include "BufferFunctions.h"

args dados;      //Flags de controle de mensagens
/*-------------------------------------------------------------------------------*/
//Trata erro de recebimento e envia
void falha(unsigned char data[]){
  limpa_string(data,63);
  if(dados.sequencia==15)
    dados.sequencia=0;
  else
    dados.sequencia++;
  if(dados.last_seq==15)   //Confere se chegou no final da dados.sequencia
    data[0]=0;    //Reinicia sequencia
  else
    data[0]=dados.last_seq+1;     //A sequencia esperada é a próxima
  memcpy(data,dados.last_data,63);
  dados.last_type=NACK;
  constroi_buffer(dados.soquete,dados.sequencia,data,NACK,1);
}
/*-------------------------------------------------------------------------------*/
//Recebe o comando cd, interpreta e envia mensagem ao cliente
void cd_command(int tipo,unsigned char data[]){
  data[dados.tamanho] = '\0';
  if(dados.aux==dados.last_seq){
    limpa_string(data,63);
    if(dados.sequencia==15)
        dados.sequencia=0;
    else
        dados.sequencia++;
    memcpy(data,dados.last_data,63);
    data[dados.tamanho] = '\0';
    if(dados.last_type==OK)
      constroi_buffer(dados.soquete,dados.sequencia,data,OK,0);
    else
      constroi_buffer(dados.soquete,dados.sequencia,data,ERRO,1);
  }
  else{
    if(dados.sequencia==15)
        dados.sequencia=0;
    else
        dados.sequencia++;
    if(!chdir(data)){
      memcpy(data,dados.last_data,63);
      dados.last_type=OK;
      constroi_buffer(dados.soquete,dados.sequencia,data,OK,0);
    }
    else{
      switch (errno){
      case EACCES:
      case EFAULT:
        data[0]='B';    // retorna que não possui permissão de acesso
        break;
      case ENOTDIR:
      case ENOENT:
        data[0]='A';    // retorna que o diretório já exite
        break;
      default:
        data[0]='Z';    // erros que não foram definidos em sala
        break;
      }
      memcpy(dados.last_data,data,63);
      dados.last_type=ERRO;
      constroi_buffer(dados.soquete,dados.sequencia,data,ERRO,1);
    }
  }
  if(dados.sequencia==16)
    dados.sequencia=0;  
}
/*-------------------------------------------------------------------------------*/
//Recebe o comando mkdir, interpreta e envia mensagem ao cliente
void mkdir_command(int tipo,unsigned char data[]){
  data[dados.tamanho] = '\0';
  if(dados.aux==dados.last_seq){
    limpa_string(data,63);
    if(dados.sequencia==15)
      dados.sequencia=0;
    else
      dados.sequencia++;
    memcpy(data,dados.last_data,63);
    data[dados.tamanho] = '\0';
    dados.last_type=OK;
    constroi_buffer(dados.soquete,dados.sequencia,data,OK,0);
  }
  else{
    if(dados.sequencia==15)
      dados.sequencia=0;
    else
      dados.sequencia++;
    if(!mkdir(data,0700)){
      memcpy(dados.last_data,data,63);
      dados.last_type=OK;
      constroi_buffer(dados.soquete,dados.sequencia,data,OK,0);
    }
    else{
      switch (errno){
      case EACCES:
      case EFAULT:
        data[0]='B';    // retorna que não possui permissão de acesso
        break;
      case EEXIST:
        data[0]='C';    // diretório já existe
        break;
      case ENOSPC:
        data[0]='E';    // retorna que não há espaço em disco para criar diretório
        break; 
      default:
        data[0]='Z';    // erros que não foram definidos em sala
        break;
      }
      memcpy(dados.last_data,data,63);
      dados.last_type=ERRO;
      constroi_buffer(dados.soquete,dados.sequencia,data,ERRO,1);
    }
  }
  if(dados.sequencia==16)
    dados.sequencia=0;
}
/*-------------------------------------------------------------------------------*/
//Recebe o comando ls, interpreta e envia arquivo contendo a listagem do diretório

void file_reader(unsigned char arq[]){
  FILE *fileptr;
  long filelen;
  int FIM_ENVIADO=0;
  int msgs=0;
  int window=0;
  int count=0;
  int tipo_recebido=DEFAULT;
  int sqc=0;
  int sqc_recv=0;
  int old_count=0;
  int old_filelen=0;
  unsigned char placeholder[63];
  unsigned char parcela[63];
  unsigned char buffer[BYTES];

  fileptr = fopen(arq,"rb");
  fseek(fileptr,0,SEEK_END);
  filelen=ftell(fileptr);
  rewind(fileptr);
  while(1){
    sqc=dados.sequencia;
    old_count=count;
    old_filelen=filelen;    
    while(filelen>0 && window<4){
      if(filelen>=63){
        fread(parcela,sizeof(char),63,fileptr);
        count+=63;
        filelen-=63;
      }
      else{
        fread(parcela,sizeof(char),filelen,fileptr);
        count+=filelen;
        filelen-=filelen;
      }
      window++;
      if(dados.sequencia==15)
        dados.sequencia=0;
      else
        dados.sequencia++;
      constroi_buffer(dados.soquete,dados.sequencia,parcela,MOSTRA,strlen(parcela));
      limpa_string(parcela,63);
    }
    if(window<4 && filelen<=0){
      if(dados.sequencia==15)
        dados.sequencia=0;
      else
        dados.sequencia++;
      FIM_ENVIADO=1;
      constroi_buffer(dados.soquete,dados.sequencia,parcela,FIM,0);
    }
    window=0;
    while(1){
      dados.buflen=recvfrom(dados.soquete,buffer,BYTES,0,NULL,0);
      if(buffer[0]==126){
        sqc_recv=DesmontaBuffer(buffer,placeholder,&tipo_recebido,&dados.last_seq,&dados.aux);  
        if(tipo_recebido==NACK || (tipo_recebido==ACK && sqc_recv!=dados.sequencia)){
          FIM_ENVIADO=0;
          filelen=old_filelen;
          count=old_count;
          dados.sequencia=sqc;
          fseek(fileptr,-count,SEEK_CUR);
          break;
        }
        if(tipo_recebido==ACK && sqc_recv==dados.sequencia){
          if(FIM_ENVIADO){
            fclose(fileptr);
            return;
          }
          else{
            break;
          }
            
        }
      }
    }
  } 
  fclose(fileptr);
}        


void ls_command(int tipo,unsigned char data[]){
  unsigned char *buffer;
  if(data[0]==0)
    system("ls > dados.txt");
  if(data[0]==1)
    system("ls -a > dados.txt");
  if(data[0]==2)
    system("ls -l > dados.txt");
  if(data[0]==3)
    system("ls -a -l > dados.txt");
  //Envio do arquivo contendo o ls
  file_reader("dados.txt");
  //windows_send(buffer);

  system("rm -rf dados.txt");
}
/*-------------------------------------------------------------------------------*/
//Função que trata cada tipo de mensagem recebida
void trata_tipo(int tipo,unsigned char data[]){
  switch (tipo)
  {
  case LS:
    ls_command(tipo,data);
    break;
  case CD:
    cd_command(tipo,data);
    break;
  case MKDIR:
    mkdir_command(tipo,data);
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
/*-------------------------------------------------------------------------------*/
int main(){
  int tipo=DEFAULT;
  dados.soquete = ConexaoRawSocket("enp7s0f0");
  unsigned char *buffer = (unsigned char *) malloc(BYTES); //to receive data
  memset(buffer,0,BYTES);
  unsigned char data[63];
  limpa_string(buffer,BYTES);
  limpa_string(data,63);
  dados.sequencia=-1;
  dados.last_seq=15;
  while(1)
  {  
    limpa_string(buffer,BYTES);
    limpa_string(data,63);
    dados.buflen=recvfrom(dados.soquete,buffer,BYTES,0,NULL,0);
    if(dados.buflen<0){
      fprintf(stderr,"error in reading recvfrom function\n");
      return -1;
    }
    if(buffer[0]==126){
      dados.tamanho=DesmontaBuffer(buffer,data,&tipo,&dados.last_seq,&dados.aux);
      if(dados.tamanho!=DEFAULT)
        trata_tipo(tipo,data);
      else
        falha(data);
    }
  }
  return 0;
}