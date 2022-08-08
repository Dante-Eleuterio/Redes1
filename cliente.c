#include "header.h"
#include "BufferFunctions.h"
#include <sys/time.h>
struct timeval relogio,timeout,tempo_inicial;
args dados;

int retorna_tipo(unsigned char buffer[],int *args_ls,unsigned char dir[]){

    int i=0;
    int a=0;
    int l=0;
    unsigned char cmd[7];
    unsigned char arg1[63];
    unsigned char arg2[6];
    sscanf(buffer,"%s %s %s",cmd,arg1,arg2);
    if(!strncmp(cmd,"ls",3)){
        if(!strncmp("-a",arg1,3))
            a=1;
        if(!strncmp("-l",arg1,3))
            l=1;
        if(!strncmp("-a",arg2,3))
            a=1;
        if(!strncmp("-l",arg2,3))
            l=1;
        if(a==1 && l==0)
            *args_ls=1;
        if(a==0 && l==1)
            *args_ls=2;
        if(a==1 && l==1)
            *args_ls=3;
        return LSL;
    }
    if(!strncmp("rls",cmd,3)){
        if(!strncmp("-a",arg1,3))
            a=1;
        if(!strncmp("-l",arg1,3))
            l=1;
        if(!strncmp("-a",arg2,3))
            a=1;
        if(!strncmp("-l",arg2,3))
            l=1;
        if(a==1 && l==0)
            *args_ls=1;
        if(a==0 && l==1)
            *args_ls=2;
        if(a==1 && l==1)
            *args_ls=3;
        return LS;
    }
    if(!strncmp("cd",cmd,3)){
        strncpy(dir,arg1,63);
        return CDL;
    }
    if(!strncmp("rcd",cmd,3)){
        strncpy(dir,arg1,63);
        return CD;
    }
    if(!strncmp("mkdir",cmd,6)){
        strncpy(dir,arg1,63);
        return MKDIRL;
    }
    if(!strncmp("rmkdir",cmd,6)){
        strncpy(dir,arg1,63);
        return MKDIR;
    }
    else
        return 0;
}
//enp7s0f0

void list_local(int *args_ls){
    if(*args_ls==0)
        system("ls");
    if(*args_ls==1)
        system("ls -a");
    if(*args_ls==2)
        system("ls -l");
    if(*args_ls==3)
        system("ls -a -l");
    *args_ls=0;
}
void list_remoto(unsigned char input[],int *args_ls,int tipo){
    int window=0;
    input[0]=*args_ls;
    *args_ls=0;
    unsigned char buffer[BYTES];
    unsigned char aux[63];
    int tipo_recebido=DEFAULT;
    int size=0;
    dados.sequencia++;
    constroi_buffer(dados.soquete,dados.sequencia,input,tipo,1);
    while(tipo_recebido!=FIM){
        gettimeofday(&tempo_inicial,NULL);
        while(tipo_recebido!=FIM){
            limpa_string(aux,63);
            dados.buflen=recvfrom(dados.soquete,buffer,BYTES,0,NULL,0);
            if(errno!=11 && dados.buflen<0){
                fprintf(stderr,"error in reading recvfrom function\n");
                fprintf(stderr,"%s\n",strerror(errno));
                exit(-1);
            }
            if(buffer[0]==126){
                size=DesmontaBuffer(buffer,aux,&tipo_recebido,&dados.last_seq,&dados.aux);
            }
            if(tipo_recebido==NACK){
                dados.tentativas=0;
                window=0;
                limpa_string(input,BYTES);
                if(dados.last_seq==15)
                    input[0]=0;
                else
                    input[0]=dados.last_seq+1;
                tipo=NACK;
                break;
            }
            
            if(tipo_recebido==FIM){
                limpa_string(input,BYTES);
                input[0]=dados.last_seq;
                dados.sequencia++;
                tipo=ACK;
                constroi_buffer(dados.soquete,dados.sequencia,input,tipo,1);
            }

            if(tipo_recebido==MOSTRA){
                window++;
                dados.tentativas=0;
                for(int i=0;i<size;i++)
                    fprintf(stderr,"%c",aux[i]);
                if(window==4){
                    window=0;
                    limpa_string(input,BYTES);
                    input[0]=dados.last_seq;
                    dados.sequencia++;
                    tipo=ACK;
                    constroi_buffer(dados.soquete,dados.sequencia,input,tipo,1);
                }
            }
            gettimeofday(&relogio,NULL);
            if(relogio.tv_sec-tempo_inicial.tv_sec>=15 && tipo_recebido!=FIM && tipo_recebido!=MOSTRA){
                fprintf(stderr,"TIMEOUT\n");
                dados.tentativas++;
                break;
            }
            memset(buffer,0,BYTES);
        }
        if(dados.tentativas==2){
            dados.tentativas=0;
            fprintf(stderr,"Tempo de espera excedido. Por favor tente novamente\n");
            break;
        }
        if(tipo_recebido!=FIM){
            tipo_recebido=DEFAULT;
            constroi_buffer(dados.soquete,dados.sequencia,input,tipo,1);
        }
    }
    dados.tentativas=0;
    tipo_recebido=DEFAULT; 
}
void cd_remoto(unsigned char dir[],int tipo){
    unsigned char buffer[BYTES];
    unsigned char aux[63];
    int tipo_recebido=DEFAULT;
    dados.sequencia++;
    constroi_buffer(dados.soquete,dados.sequencia,dir,tipo,strlen(dir));
    while(tipo_recebido!=OK){
        gettimeofday(&tempo_inicial,NULL);
        while(tipo_recebido!=OK){
            limpa_string(aux,63);
            dados.buflen=recvfrom(dados.soquete,buffer,BYTES,0,NULL,0);
            if(errno!=11 && dados.buflen<0){
                fprintf(stderr,"error in reading recvfrom function\n");
                fprintf(stderr,"%s\n",strerror(errno));
                exit(-1);
            }
            if(buffer[0]==126){
                DesmontaBuffer(buffer,aux,&tipo_recebido,&dados.last_seq,&dados.aux);
            }
            if(tipo_recebido==NACK){
                dados.tentativas=0;
                limpa_string(aux,63);
                break;
            }
            if(tipo_recebido==ERRO){
                switch (aux[0])
                {
                case 'A':
                    fprintf(stderr,"Diretorio inexistente\n");
                    break;
                case 'B':
                    fprintf(stderr,"Sem permissao de Acesso\n");
                    break;                            
                default:
                    fprintf(stderr,"Erro Desconhecido\n");
                    break;
                }
                break;
            }
            gettimeofday(&relogio,NULL);
            if(relogio.tv_sec-tempo_inicial.tv_sec>=15 && tipo_recebido!=OK){
                fprintf(stderr,"TIMEOUT\n");
                dados.tentativas++;
                break;
            }
            memset(buffer,0,BYTES);
        }
        if(tipo_recebido==ERRO){
            break;
        }
        if(dados.tentativas==2){
            dados.tentativas=0;
            fprintf(stderr,"Tempo de espera excedido. Por favor tente novamente\n");
            break;
        }
        if(tipo_recebido!=OK){
            tipo_recebido=DEFAULT;
            constroi_buffer(dados.soquete,dados.sequencia,dir,tipo,strlen(dir));
        }

    }
    dados.tentativas=0;
    tipo_recebido=DEFAULT; 

}

void mkdir_remoto(unsigned char dir[],int tipo){
    unsigned char buffer[BYTES];
    unsigned char aux[63];
    int tipo_recebido=DEFAULT;
    dados.sequencia++;
    constroi_buffer(dados.soquete,dados.sequencia,dir,tipo,strlen(dir));
    while(tipo_recebido!=OK){
        gettimeofday(&tempo_inicial,NULL);
        while(tipo_recebido!=OK){
            limpa_string(aux,63);
            memset(buffer,0,BYTES);
            dados.buflen=recvfrom(dados.soquete,buffer,BYTES,0,NULL,0);
            if(errno!=11 && dados.buflen<0){
                fprintf(stderr,"error in reading recvfrom function\n");
                exit(-1);
            }
            if(buffer[0]==126){
                DesmontaBuffer(buffer,aux,&tipo_recebido,&dados.last_seq,&dados.aux);
            }
            if(tipo_recebido==NACK){
                dados.tentativas=0;
                limpa_string(aux,63);
                break;
            }
            if(tipo_recebido==ERRO){
                switch (aux[0])
                {
                case 'C':
                    fprintf(stderr,"Diretorio ja existente\n");
                    break;
                case 'B':
                    fprintf(stderr,"Sem permissao de Acesso\n");
                    break;                            
                case 'E':
                    fprintf(stderr,"Nao ha espaco para criar o diretorio\n");
                    break;                            
                default:
                    fprintf(stderr,"Erro Desconhecido\n");
                    break;
                }
                break;
            }
            gettimeofday(&relogio,NULL);
            if(relogio.tv_sec-tempo_inicial.tv_sec>=10 && tipo_recebido!=OK){
                fprintf(stderr,"TIMEOUT\n");
                dados.tentativas++;
                break;
            }
        }
        if(tipo_recebido==ERRO){
            break;
        }
        if(dados.tentativas==2){
            dados.tentativas=0;
            fprintf(stderr,"Tempo de espera excedido. Por favor tente novamente\n");
            break;
        }
        if(tipo_recebido!=OK){
            tipo_recebido=DEFAULT;
            constroi_buffer(dados.soquete,dados.sequencia,dir,tipo,strlen(dir));
        }

    }
    dados.tentativas=0;
    tipo_recebido=DEFAULT;                
}

void inicializa(int *tipo,int *args_ls){
    dados.tentativas=0;
    dados.buflen=0;
    dados.last_seq=15;
    dados.sequencia=-1;
    dados.aux=0;
    *args_ls=0;
    *tipo=0;
}
void envia(unsigned char buffer[63]){
    unsigned char input[BYTES],aux[63],dir[63];
    int tipo,args_ls;
    inicializa(&tipo,&args_ls);
    while(1){
        limpa_string(input,BYTES);
        limpa_string(dir,63);
        limpa_string(aux,63);
        char cwd[PATH_MAX];
        fprintf(stderr,"Cliente:%s$ ",getcwd(cwd, sizeof(cwd)));
        fgets(input,BYTES,stdin);
        tipo=retorna_tipo(input,&args_ls,dir);
        limpa_string(input,BYTES);
        switch (tipo)
        {
            case LSL:
                list_local(&args_ls);
                break;
            case LS:
                list_remoto(input,&args_ls,tipo);
                break;
            case CDL:
                chdir(dir);
                break;
            case CD:
                cd_remoto(dir,tipo);       
                break;
            case MKDIRL:
                mkdir(dir,0700);
                break;
            case MKDIR:
                mkdir_remoto(dir,tipo);       
                break;
            default:
                dados.sequencia++;
                constroi_buffer(dados.soquete,dados.sequencia,input,tipo,strlen(input));
                break;
        }
        if (dados.sequencia==15)
            dados.sequencia=0;
    }
}




int main(int argc, char const *argv[]){
    timeout.tv_sec=5;
    unsigned char *buffer = (unsigned char *) malloc(BYTES); //to receive data
    memset(buffer,0,BYTES);
    dados.soquete= ConexaoRawSocket("enp7s0f0");
    if(dados.soquete<0)
    {
        fprintf(stderr,"error in sending....sendlen=%d....errno=%d\n",dados.soquete,errno);
        return -1;
    }
    setsockopt(dados.soquete,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
    system("clear");
    envia(buffer);





  return 0;
}