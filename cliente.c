#include "header.h"
#include "BufferFunctions.h"
#include <sys/time.h>
int sequencia;
int last_seq;
int sequencia_recebida;
struct timeval relogio,timeout,tempo_inicial;


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

int main(int argc, char const *argv[])
{
    int tentativas=0;
    timeout.tv_sec=5;
    unsigned char *buffer = (unsigned char *) malloc(BYTES); //to receive data
    memset(buffer,0,BYTES);
    int tipo_recebido=DEFAULT;
    int args_ls=0;
    int buflen;
    int send_len= ConexaoRawSocket("enp7s0f0");
    if(send_len<0)
    {
        printf("error in sending....sendlen=%d....errno=%d\n",send_len,errno);
        return -1;
    }
    setsockopt(send_len,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
    unsigned char input[76];
    unsigned char dir[63];
    unsigned char aux[63];
    int tipo=0;
    char cwd[PATH_MAX];
    sequencia=-1;
    last_seq=15;
    system("clear");
    while(1){
        limpa_string(input,76);
        limpa_string(dir,63);
        limpa_string(aux,63);
        printf("Cliente:%s$ ",getcwd(cwd, sizeof(cwd)));
        fgets(input,76,stdin);
        tipo=retorna_tipo(input,&args_ls,dir);
        limpa_string(input,76);
        switch (tipo)
        {
            case LSL:
            {
                if(args_ls==0)
                    system("ls");
                if(args_ls==1)
                    system("ls -a");
                if(args_ls==2)
                    system("ls -l");
                if(args_ls==3)
                    system("ls -a -l");
                args_ls=0;
                break;
            }
            case LS:
                input[0]=args_ls;
                sequencia++;
                constroi_buffer(send_len,sequencia,input,tipo,1);
                args_ls=0;
                break;
            case CDL:
                chdir(dir);
                break;
            case CD:
                sequencia++;
                constroi_buffer(send_len,sequencia,dir,tipo,strlen(dir));
                while(tipo_recebido!=OK){
                    gettimeofday(&tempo_inicial,NULL);
                    while(tipo_recebido!=OK){
                        limpa_string(aux,63);
                        buflen=recvfrom(send_len,buffer,BYTES,0,NULL,0);
                        if(errno!=11 && buflen<0){
                            printf("error in reading recvfrom function\n");
                            return -1;
                        }
                        if(buffer[0]==126){
                            DesmontaBuffer(buffer,aux,&tipo_recebido,&last_seq,&sequencia_recebida);
                        }
                        if(tipo_recebido==NACK){
                            tentativas=0;
                            limpa_string(aux,63);
                            break;
                        }
                        if(tipo_recebido==ERRO){
                            switch (aux[0])
                            {
                            case 'A':
                                printf("Diretorio inexistente\n");
                                break;
                            case 'B':
                                printf("Sem permissao de Acesso\n");
                                break;                            
                            default:
                                printf("Erro Desconhecido\n");
                                break;
                            }
                            break;
                        }
                        gettimeofday(&relogio,NULL);
                        if(relogio.tv_sec-tempo_inicial.tv_sec>=15 && tipo_recebido!=OK){
                            printf("TIMEOUT\n");
                            tentativas++;
                            break;
                        }
                        memset(buffer,0,BYTES);
                    }
                    if(tipo_recebido==ERRO){
                        break;
                    }
                    if(tentativas==2){
                        tentativas=0;
                        printf("Tempo de espera excedido. Por favor tente novamente\n");
                        break;
                    }
                    if(tipo_recebido!=OK){
                        tipo_recebido=DEFAULT;
                        constroi_buffer(send_len,sequencia,dir,tipo,strlen(dir));
                    }

                }
                tentativas=0;
                tipo_recebido=DEFAULT;                
                break;
            case MKDIRL:
                mkdir(dir,0700);
                break;
            case MKDIR:
                sequencia++;
                constroi_buffer(send_len,sequencia,dir,tipo,strlen(dir));
                while(tipo_recebido!=OK){
                    gettimeofday(&tempo_inicial,NULL);
                    while(tipo_recebido!=OK){
                        limpa_string(aux,63);
                        buflen=recvfrom(send_len,buffer,BYTES,0,NULL,0);
                        if(errno!=11 && buflen<0){
                            printf("error in reading recvfrom function\n");
                            return -1;
                        }
                        if(buffer[0]==126){
                            DesmontaBuffer(buffer,aux,&tipo_recebido,&last_seq,&sequencia_recebida);
                        }
                        if(tipo_recebido==NACK){
                            tentativas=0;
                            limpa_string(aux,63);
                            break;
                        }
                        if(tipo_recebido==ERRO){
                            switch (aux[0])
                            {
                            case 'C':
                                printf("Diretorio ja existente\n");
                                break;
                            case 'B':
                                printf("Sem permissao de Acesso\n");
                                break;                            
                            case 'E':
                                printf("Nao ha espaco para criar o diretorio\n");
                                break;                            
                            default:
                                printf("Erro Desconhecido\n");
                                break;
                            }
                            break;
                        }
                        gettimeofday(&relogio,NULL);
                        if(relogio.tv_sec-tempo_inicial.tv_sec>=15 && tipo_recebido!=OK){
                            printf("TIMEOUT\n");
                            tentativas++;
                            break;
                        }
                        memset(buffer,0,BYTES);
                    }
                    if(tipo_recebido==ERRO){
                        break;
                    }
                    if(tentativas==2){
                        tentativas=0;
                        printf("Tempo de espera excedido. Por favor tente novamente\n");
                        break;
                    }
                    if(tipo_recebido!=OK){
                        tipo_recebido=DEFAULT;
                        constroi_buffer(send_len,sequencia,dir,tipo,strlen(dir));
                    }

                }
                tentativas=0;
                tipo_recebido=DEFAULT;                
                break;            
                break;
            default:
                sequencia++;
                constroi_buffer(send_len,sequencia,input,tipo,strlen(input));
                break;
        }
        if (sequencia==15)
            sequencia=0;
    }

                constroi_buffer(send_len,sequencia,dir,tipo,strlen(dir));




  return 0;
}