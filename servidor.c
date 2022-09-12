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
  unsigned long buffer[BYTES];

  fileptr = fopen(arq,"rb");
  fseek(fileptr,0,SEEK_END);
  filelen=ftell(fileptr);
  rewind(fileptr);
  while(1){
    // fprintf(stderr,"entrou while principal\n");
    sqc=dados.sequencia;
    old_count=count;
    old_filelen=filelen;    
    while(filelen>0 && window<4){
      // fprintf(stderr,"entrou while da janela\n");
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
      fprintf(stderr, "Enviando %d\n", dados.sequencia);
      constroi_buffer(dados.soquete,dados.sequencia,parcela,MOSTRA,size);
      size=0;
    }
    // fprintf(stderr,"saiu while da janela\n");
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
      // fprintf(stderr,"entrou while da resposta\n");
      memset(buffer,0,BYTES);
      
      dados.buflen=recv(dados.soquete,buffer,sizeof(unsigned long)*BYTES,0);
      if(errno!=11 && dados.buflen<0){
        fprintf(stderr,"error in reading recv function\n");
        exit(-1);
      }
      if(errno==11){
        fprintf(stderr,"TIMEOUT\n");
        dados.tentativas++;
        FIM_ENVIADO=0;
        filelen=old_filelen;
        dados.sequencia=sqc;
        fprintf(stderr,"ftell: %ld count:%d\n",ftell(fileptr),count);
        fseek(fileptr,-(count-old_count),SEEK_CUR);
        count=old_count;
        fprintf(stderr,"ftell: %ld count:%d\n",ftell(fileptr),count);
        break;
      }

      if(buffer[0]==126){
        // fprintf(stderr,"entrou if do desmonta\n");
        sqc_recv=DesmontaBuffer(buffer,placeholder,&tipo_recebido,&dados.last_seq,&dados.aux);  
        if(sqc_recv!=DEFAULT){
          if(tipo_recebido==ACK && sqc_recv==dados.sequencia){
            fprintf(stderr, "Recebi ACK\n");
            if(FIM_ENVIADO){
              fclose(fileptr);
              fprintf(stderr,"caiu fora 1\n");
              return;
            }
            break;
          }

          if(tipo_recebido==NACK || (tipo_recebido==ACK && sqc_recv!=dados.sequencia)){
            fprintf(stderr,"NACK\n");
            FIM_ENVIADO=0;
            filelen=old_filelen;
            dados.sequencia=sqc;
            fseek(fileptr,-(count-old_count),SEEK_CUR);
            count=old_count;
            break;
          }
        }
        else
          fprintf(stderr,"DEU MERDA AQUI \n");
      } else fprintf(stderr,"Recebi um protocolo invalido\n");
    }
    // fprintf(stderr,"saiu while da resposta\n");
  } 
  fclose(fileptr);
  fprintf(stderr,"caiu fora 2\n");

}        



void ls_command(int tipo,unsigned char data[]){
  int ret,status;
  ret=fork();
  if(ret==0){
    if(data[0]==0)
      system("ls > .dados.txt");
    if(data[0]==1)
      system("ls -a > .dados.txt");
    if(data[0]==2)
      system("ls -l > .dados.txt");
    if(data[0]==3)
      system("ls -a -l > .dados.txt");
    exit(0);
  }
  else
    wait(&status);
  //Envio do arquivo contendo o ls
  file_reader(".dados.txt");
  fprintf(stderr,"voltou pra ls_command\n");

  system("rm -f .dados.txt");
}

void get_command(int tipo,unsigned char data[]){
  if(access(data,F_OK)==0){
    unsigned char aux[16];
    unsigned char aux2[63];
    unsigned long buffer[BYTES];
    int tipo_recebido=DEFAULT;
    struct stat status;
    stat(data,&status);
    if(S_ISREG(status.st_mode)){
      sprintf(aux,"%ld",status.st_size);
      if(dados.sequencia==15)
        dados.sequencia=0;
      else
        dados.sequencia++;
      constroi_buffer(dados.soquete,dados.sequencia,aux,DESCRITOR,16);
      do{
        memset(buffer,0,BYTES);
        dados.buflen=recv(dados.soquete,buffer,sizeof(unsigned long)*BYTES,0);
        if(errno!=11 && dados.buflen<0){
          fprintf(stderr,"error in readind recv function\n");
          fprintf(stderr,"%s\n",strerror(errno));
          exit(-1);
        }
        if(buffer[0]==126){
          DesmontaBuffer(buffer,aux2,&tipo_recebido,&dados.last_seq,&dados.aux);
          if(tipo_recebido==ERRO){
            return;
          }
          if(tipo_recebido==OK){
            file_reader(data);    
            return;
          }
        }
      }while(errno==11);
    }
    else{
      limpa_string(data,63);
      data[0]='F';    // Tentou pegar um arquivo não regular
      memcpy(dados.last_data,data,63);
      dados.last_type=ERRO;
      if(dados.sequencia==15)
        dados.sequencia=0;
      else
        dados.sequencia++;
      constroi_buffer(dados.soquete,dados.sequencia,data,ERRO,1);
    }    
  }
  else{
    limpa_string(data,63);
    switch (errno){
    case EACCES:
    case EFAULT:
      data[0]='B';    // retorna que não possui permissão de acesso
      break;
    case ENOTDIR:
    case ENOENT:
      data[0]='D';    // retorna que o arquivo não exite
      break;
    default:
      data[0]='Z';    // erros que não foram definidos em sala
      break;
    }
    memcpy(dados.last_data,data,63);
    dados.last_type=ERRO;
    if(dados.sequencia==15)
      dados.sequencia=0;
    else
      dados.sequencia++;
    constroi_buffer(dados.soquete,dados.sequencia,data,ERRO,1);
  }
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
      get_command(tipo,data);
    break;
  case PUT:

    break;
  default:
    break;
  }
}
/*-------------------------------------------------------------------------------*/
int main(){
  int tipo=DEFAULT;
  timeout.tv_usec=500000;
  dados.soquete = ConexaoRawSocket("enp7s0f0");
  setsockopt(dados.soquete,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
  unsigned long *buffer = (unsigned long *) malloc(sizeof(unsigned long)*BYTES); //to receive data
  memset(buffer,0,BYTES);
  unsigned char data[63];
  limpa_string(data,63);
  dados.sequencia=15;
  dados.last_seq=15;
  while(1)
  {  
    memset(buffer,0,BYTES);
    limpa_string(data,63);
    dados.buflen=recv(dados.soquete,buffer,sizeof(unsigned long)*BYTES,0);
    if(errno!=11 && dados.buflen<0){
      fprintf(stderr,"error in reading recv function\n");
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