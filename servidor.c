#include "header.h"
#include "BufferFunctions.h"

struct timeval relogio,timeout,tempo_inicial;
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
}
/*-------------------------------------------------------------------------------*/
//Recebe o comando ls, interpreta e envia arquivo contendo a listagem do diretório

void file_reader(unsigned char arq[]){
  fprintf(stderr,"entrou aqui\n");
  FILE *fileptr;
  long filelen;
  int size=0;
  int voltou=0;
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
    fprintf(stderr,"entrou while principal\n");
    voltou=0;
    sqc=dados.sequencia;
    old_count=count;
    old_filelen=filelen;    
    while(filelen>0 && window<4){
      fprintf(stderr,"entrou while da janela\n");
      limpa_string(parcela,63);
      if(filelen>=63){
        fread(parcela,sizeof(char),63,fileptr);
        size=63;
        count+=63;
        filelen-=63;
      }
      else{
        fread(parcela,sizeof(char),filelen,fileptr);
        size=filelen;
        count+=filelen;
        filelen-=filelen;
      }
      window++;
      if(dados.sequencia==15)
        dados.sequencia=0;
      else
        dados.sequencia++;
      constroi_buffer(dados.soquete,dados.sequencia,parcela,MOSTRA,size);
      size=0;
    }
    fprintf(stderr,"saiu while da janela\n");
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
      errno=0;
      fprintf(stderr,"entrou while da resposta\n");
      limpa_string(buffer,BYTES);
      gettimeofday(&tempo_inicial,NULL);
      dados.buflen=recvfrom(dados.soquete,buffer,BYTES,0,NULL,0);
      if(errno!=11 && dados.buflen<0){
        fprintf(stderr,"error in reading recvfrom function\n");
        exit(-1);
      }
      if(errno==11){
        fprintf(stderr,"cancelado\n");
      }
      if(errno!=11 && buffer[0]==126){
        fprintf(stderr,"entrou if do desmonta\n");
        sqc_recv=DesmontaBuffer(buffer,placeholder,&tipo_recebido,&dados.last_seq,&dados.aux);  
        if(sqc_recv!=DEFAULT){
          if(!voltou && (tipo_recebido==NACK || (tipo_recebido==ACK && sqc_recv!=dados.sequencia))){
            fprintf(stderr,"entrou if volta\n");
            FIM_ENVIADO=0;
            filelen=old_filelen;
            dados.sequencia=sqc;
            fseek(fileptr,-(count-old_count),SEEK_CUR);
            count=old_count;
            voltou=1;
            break;
          }
          if(tipo_recebido==ACK && sqc_recv==dados.sequencia){
            if(FIM_ENVIADO){
              fclose(fileptr);
              fprintf(stderr,"caiu fora 1\n");
              return;
            }
            else{
              break;
            }
          }
        }
        else
          fprintf(stderr,"DEU MERDA AQUI \n");
      }
      gettimeofday(&relogio,NULL);
      if(!voltou && (relogio.tv_sec-tempo_inicial.tv_sec>=4)){
        fprintf(stderr,"TIMEOUT\n");
        dados.tentativas++;
        FIM_ENVIADO=0;
        filelen=old_filelen;
        dados.sequencia=sqc;
        fprintf(stderr,"ftell: %ld count:%d\n",ftell(fileptr),count);
        fseek(fileptr,-(count-old_count),SEEK_CUR);
        count=old_count;
        fprintf(stderr,"ftell: %ld count:%d\n",ftell(fileptr),count);
        voltou=1;
        break;
      }
    }
    fprintf(stderr,"saiu while da resposta\n");
  } 
  fclose(fileptr);
  fprintf(stderr,"caiu fora 2\n");

}        


void ls_command(int tipo,unsigned char data[]){
  unsigned char *buffer;
  int ret,status;
  ret=fork();
  if(ret==0){
    if(data[0]==0)
      system("ls > dados.txt");
    if(data[0]==1)
      system("ls -a > dados.txt");
    if(data[0]==2)
      system("ls -l > dados.txt");
    if(data[0]==3)
      system("ls -a -l > dados.txt");
    exit(0);
  }
  else
    wait(&status);
  //Envio do arquivo contendo o ls
  file_reader("FLOR.jpg");
  fprintf(stderr,"voltou pra ls_command\n");
  //windows_send(buffer);

  //system("rm -f dados.txt");
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
  timeout.tv_sec=2;
  dados.soquete = ConexaoRawSocket("enp7s0f0");
  setsockopt(dados.soquete,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
  unsigned char *buffer = (unsigned char *) malloc(BYTES); //to receive data
  memset(buffer,0,BYTES);
  unsigned char data[63];
  limpa_string(buffer,BYTES);
  limpa_string(data,63);
  dados.sequencia=15;
  dados.last_seq=15;
  while(1)
  {  
    limpa_string(buffer,BYTES);
    limpa_string(data,63);
    dados.buflen=recvfrom(dados.soquete,buffer,BYTES,0,NULL,0);
    if(errno!=11 && dados.buflen<0){
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