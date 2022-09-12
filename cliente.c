#include "header.h"
#include "BufferFunctions.h"
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
    if(!strncmp("get",cmd,3)){
        strncpy(dir,arg1,63);
        return GET;
    }
    else
        return 0;
}

void get(unsigned char input[],int tipo){
    FILE* arq ;
    arq = fopen (input, "w") ;
    int ret=0;
    int window=0;
    unsigned long buffer[BYTES];
    unsigned char aux[63];
    int tipo_recebido=DEFAULT;
    int size=0;
    int recebeu=0;
    if (dados.sequencia==15)
        dados.sequencia=0;
    else
        dados.sequencia++;
    constroi_buffer(dados.soquete,dados.sequencia,input,tipo,strnlen(input,63));
    limpa_string(aux,63);
    memset(buffer,0,BYTES);
    errno=0;
    do{
        dados.buflen=recv(dados.soquete,buffer,sizeof(unsigned long)*BYTES,0);
        if(errno!=11 && dados.buflen<0){
            fprintf(stderr,"error in reading recv function\n");
            fprintf(stderr,"%s\n",strerror(errno));
            exit(-1);
        }
        if(errno!=11 && buffer[0]==126){
            size=DesmontaBuffer(buffer,aux,&tipo_recebido,&dados.last_seq,&dados.aux);
            if(tipo_recebido==DESCRITOR){
                fprintf(stderr,"oi mundo\n");   
                long mem_livre,tamanho=0;
                char pwd[63];
                tamanho= atoi(aux);
                getcwd(pwd,sizeof(pwd));
                char *bashdf = "df --output=avail --block-size=1 / | tail -n +2 > /tmp/tamanho_gb.txt";
                system(bashdf);
                FILE *memfree = fopen("/tmp/tamanho_gb.txt","r");
                fscanf(memfree,"%ld",&mem_livre);
                fclose(memfree);
                system("rm /tmp/tamanho_gb.txt");
                if (dados.sequencia==15)
                    dados.sequencia=0;
                else
                    dados.sequencia++;
                if(tamanho>mem_livre){
                    limpa_string(aux,63);
                    aux[0]='M';
                    fprintf(stderr,"Nao ha memoria o suficiente para a transferencia\n");
                    constroi_buffer(dados.soquete,dados.sequencia,aux,ERRO,1);
                    return;
                }
                else{
                    limpa_string(aux,63);
                    constroi_buffer(dados.soquete,dados.sequencia,aux,OK,0);
                    recebeu=1;
                }

            }
            if(tipo_recebido==ERRO){
                switch (aux[0])
                {
                case 'D':
                    fprintf(stderr,"Arquivo inexistente\n");
                    break;
                case 'B':
                    fprintf(stderr,"Sem permissao de Acesso\n");
                    break;
                case 'F':
                    fprintf(stderr,"Arquivo nao regular\n");
                    break;
                default:
                    fprintf(stderr,"Erro Desconhecido\n");
                    break;
                }
                return;
            }
        }
    }while(errno==11 ||!recebeu);

    while(tipo_recebido!=FIM){
        while(tipo_recebido!=FIM){
            limpa_string(aux,63);
            memset(buffer,0,BYTES);
            errno=0;
            // fprintf(stderr, "lendo proxima\n");
            dados.buflen=recv(dados.soquete,buffer,sizeof(unsigned long)*BYTES,0);
            if(errno!=11 && dados.buflen<0){
                fprintf(stderr,"error in reading recv function\n");
                fprintf(stderr,"%s\n",strerror(errno));
                exit(-1);
            }

            // fprintf(stderr, "errno = %d buffer = ", errno);
            // for (int i = 0; i < dados.buflen; i++) {
            //     unsigned char d = buffer[i];
            //     if (d < 0x20 || d > 0xf0) {
            //         fprintf(stderr,"[%d]", d);
            //     } else {
            //         fprintf(stderr,"'%c'", d);
            //     }
            // }
            // fprintf(stderr, "\n");

            if(errno!=11 && buffer[0]==126){
                size=DesmontaBuffer(buffer,aux,&tipo_recebido,&dados.last_seq,&dados.aux);
                // fprintf(stderr, "seq = %d tipo recebido = %d size = %d\n", dados.last_seq, tipo_recebido, size);
                if(size!=FEITO){
                    
                    if(tipo_recebido==NACK){
                        dados.tentativas=0;
                        window=0;
                        memset(buffer,0,BYTES);
                        if(dados.last_seq==15)
                            input[0]=0;
                        else
                            input[0]=dados.last_seq+1;
                        // if (dados.sequencia==15)
                        //     dados.sequencia=0;
                        // else
                        //     dados.sequencia++;
                        tipo=NACK;
                        constroi_buffer(dados.soquete,dados.sequencia,input,tipo,1);
                        break;
                    }
                    
                    if(tipo_recebido==FIM){
                        window=0;
                        memset(buffer,0,BYTES);
                        input[0]=dados.last_seq;
                        if (dados.sequencia==15)
                            dados.sequencia=0;
                        else
                            dados.sequencia++;
                        tipo=ACK;
                        constroi_buffer(dados.soquete,dados.sequencia,input,tipo,1);
                        fprintf(stderr,"\n");
                    }

                    if(tipo_recebido==MOSTRA){
                        window++;
                        dados.tentativas=0;
                        ret = fwrite (aux, size,1 , arq) ;

                        if(window==4){
                            window=0;
                            memset(buffer,0,BYTES);
                            input[0]=dados.last_seq;
                            if (dados.sequencia==15)
                                dados.sequencia=0;
                            else
                                dados.sequencia++;
                            tipo=ACK;
                            // fprintf(stderr, "Enviando ACK\n");
                            constroi_buffer(dados.soquete,dados.sequencia,input,tipo,1);
                        }
                    }
                } 
            } 
        }
        fprintf(stderr, "ok\n");
        
    }
    fclose(arq);
}



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
    // FILE* arq ;
    // arq = fopen ("rickrollcopia.mp3", "w") ;
    int ret=0;
    int window=0;
    input[0]=*args_ls;
    *args_ls=0;
    unsigned long buffer[BYTES];
    unsigned char aux[63];
    int tipo_recebido=DEFAULT;
    int size=0;
    if (dados.sequencia==15)
        dados.sequencia=0;
    else
        dados.sequencia++;
    constroi_buffer(dados.soquete,dados.sequencia,input,tipo,1);
    while(tipo_recebido!=FIM){
        while(tipo_recebido!=FIM){
            limpa_string(aux,63);
            memset(buffer,0,BYTES);
            errno=0;
            // fprintf(stderr, "lendo proxima\n");
            dados.buflen=recv(dados.soquete,buffer,sizeof(unsigned long)*BYTES,0);
            if(errno!=11 && dados.buflen<0){
                fprintf(stderr,"error in reading recv function\n");
                fprintf(stderr,"%s\n",strerror(errno));
                exit(-1);
            }

            // fprintf(stderr, "errno = %d buffer = ", errno);
            // for (int i = 0; i < dados.buflen; i++) {
            //     unsigned char d = buffer[i];
            //     if (d < 0x20 || d > 0xf0) {
            //         fprintf(stderr,"[%d]", d);
            //     } else {
            //         fprintf(stderr,"'%c'", d);
            //     }
            // }
            // fprintf(stderr, "\n");

            if(errno!=11 && buffer[0]==126){
                size=DesmontaBuffer(buffer,aux,&tipo_recebido,&dados.last_seq,&dados.aux);
                // fprintf(stderr, "seq = %d tipo recebido = %d size = %d\n", dados.last_seq, tipo_recebido, size);
                if(size!=FEITO){
                    if(tipo_recebido==NACK){
                        dados.tentativas=0;
                        window=0;
                        memset(buffer,0,BYTES);
                        if(dados.last_seq==15)
                            input[0]=0;
                        else
                            input[0]=dados.last_seq+1;
                        // if (dados.sequencia==15)
                        //     dados.sequencia=0;
                        // else
                        //     dados.sequencia++;
                        tipo=NACK;
                        constroi_buffer(dados.soquete,dados.sequencia,input,tipo,1);
                        break;
                    }
                    
                    if(tipo_recebido==FIM){
                        window=0;
                        memset(buffer,0,BYTES);
                        input[0]=dados.last_seq;
                        if (dados.sequencia==15)
                            dados.sequencia=0;
                        else
                            dados.sequencia++;
                        tipo=ACK;
                        constroi_buffer(dados.soquete,dados.sequencia,input,tipo,1);
                        fprintf(stderr,"\n");
                    }

                    if(tipo_recebido==MOSTRA){
                        window++;
                        dados.tentativas=0;
                        // ret = fwrite (aux, size,1 , arq) ;
                        fprintf(stderr,"%s",aux);
                        if(window==4){
                            window=0;
                            memset(buffer,0,BYTES);
                            input[0]=dados.last_seq;
                            if (dados.sequencia==15)
                                dados.sequencia=0;
                            else
                                dados.sequencia++;
                            tipo=ACK;
                            // fprintf(stderr, "Enviando ACK\n");
                            constroi_buffer(dados.soquete,dados.sequencia,input,tipo,1);
                        }
                    }
                } 
            } 
        }
        // fprintf(stderr, "ok\n");
        
    }
    dados.tentativas=0;
    tipo_recebido=DEFAULT; 
    // fclose(arq);
}
void cd_remoto(unsigned char dir[],int tipo){
    unsigned long buffer[BYTES];
    unsigned char aux[63];
    int tipo_recebido=DEFAULT;
    if (dados.sequencia==15)
        dados.sequencia=0;
    else
        dados.sequencia++;
    constroi_buffer(dados.soquete,dados.sequencia,dir,tipo,strlen(dir));
    while(tipo_recebido!=OK){
        gettimeofday(&tempo_inicial,NULL);
        while(tipo_recebido!=OK){
            limpa_string(aux,63);
            dados.buflen=recv(dados.soquete,buffer,sizeof(unsigned long)*BYTES,0);
            if(errno!=11 && dados.buflen<0){
                fprintf(stderr,"error in reading recv function\n");
                fprintf(stderr,"%s\n",strerror(errno));
                exit(-1);
            }
            if(buffer[0]==126){
                if(DesmontaBuffer(buffer,aux,&tipo_recebido,&dados.last_seq,&dados.aux)==FEITO){
                    tipo_recebido=DEFAULT;
                    break;
                }
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
    unsigned long buffer[BYTES];
    unsigned char aux[63];
    int tipo_recebido=DEFAULT;
    if (dados.sequencia==15)
        dados.sequencia=0;
    else
        dados.sequencia++;
    constroi_buffer(dados.soquete,dados.sequencia,dir,tipo,strlen(dir));
    while(tipo_recebido!=OK){
        gettimeofday(&tempo_inicial,NULL);
        while(tipo_recebido!=OK){
            limpa_string(aux,63);
            memset(buffer,0,BYTES);
            dados.buflen=recv(dados.soquete,buffer,sizeof(unsigned long)*BYTES,0);
            if(errno!=11 && dados.buflen<0){
                fprintf(stderr,"error in reading recv function\n");
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
    dados.sequencia=15;
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
            case GET:
                get(dir,tipo);
                break;
            default:
                break;
        }
    }
}
int main(int argc, char const *argv[]){
    timeout.tv_usec=500000;
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
    free(buffer);




  return 0;
}