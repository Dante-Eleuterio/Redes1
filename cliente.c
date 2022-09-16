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
    if(!strncmp("put",cmd,3)){
        strncpy(dir,arg1,63);
        return PUT;
    }
    else
        return 0;
}
void file_reader(unsigned char arq[]){
    FILE *fileptr;
    double filelen;
    int size=0;
    double total=0;
    int percentual=100;
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
    total=filelen;
    rewind(fileptr);
    while(1){
    sqc=dados.sequencia;
    old_count=count;
    old_filelen=filelen;    
    while(filelen>0 && window<4){
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
        memset(buffer,0,BYTES);
        
        dados.buflen=recv(dados.soquete,buffer,sizeof(unsigned long)*BYTES,0);
        if(errno!=11 && dados.buflen<0){
            fprintf(stderr,"error in reading recv function\n");
            exit(-1);
        }
        if(errno==11){
            fprintf(stderr,"\nTIMEOUT\n");
            FIM_ENVIADO=0;
            filelen=old_filelen;
            dados.sequencia=sqc;
            fseek(fileptr,-(count-old_count),SEEK_CUR);
            count=old_count;
            break;
        }

        if(buffer[0]==126){
            sqc_recv=DesmontaBuffer(buffer,placeholder,&tipo_recebido,&dados.last_seq,&dados.aux);  
            if(sqc_recv!=DEFAULT){
                if(tipo_recebido==ACK && sqc_recv==dados.sequencia){
                    if(floor((filelen/total)*100)!=percentual){
                        percentual--;
                        fprintf(stderr, "\r%.0f%c",floor((filelen/total)*100),37);
                        fflush(stderr);
                    }
                if(FIM_ENVIADO){
                    fclose(fileptr);
                    fprintf(stderr,"\n");
                    return;
                }
                break;
                }

                if(tipo_recebido==NACK || (tipo_recebido==ACK && sqc_recv!=dados.sequencia)){
                FIM_ENVIADO=0;
                filelen=old_filelen;
                dados.sequencia=sqc;
                fseek(fileptr,-(count-old_count),SEEK_CUR);
                count=old_count;
                break;
                }
            }
        }
    }
    } 
    fclose(fileptr);

}        

void put(unsigned char input[],int tipo){
    if(access(input,F_OK)==0){
        unsigned char aux[16];
        unsigned char aux2[63];
        unsigned long buffer[BYTES];
        int tipo_recebido=DEFAULT;
        struct stat status;
        stat(input,&status);
        if(S_ISREG(status.st_mode)){
            if(dados.sequencia==15)
                dados.sequencia=0;
            else
                dados.sequencia++;
            constroi_buffer(dados.soquete,dados.sequencia,input,PUT,strnlen(input,63));
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
                    if(tipo_recebido==OK){
                        sprintf(aux,"%ld",status.st_size);
                        if(dados.sequencia==15)
                            dados.sequencia=0;
                        else
                            dados.sequencia++;
                        constroi_buffer(dados.soquete,dados.sequencia,aux,DESCRITOR,16);
                        break;
                    }
                }
            }while(1);
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
                       switch (aux[0])
                        {
                        case 'B':
                            fprintf(stderr,"Sem permissao de Acesso\n");
                            break;
                        case 'M':
                            fprintf(stderr,"Sem memoria disponivel\n");
                            break;
                        default:
                            fprintf(stderr,"Erro Desconhecido\n");
                            break;
                        }
                        return;
                    }
                    if(tipo_recebido==OK){
                        file_reader(input);    
                        return;
                    }
                }
            }while(1); 
        }
        else{
            fprintf(stderr,"Arquivo nao regular\n");
            return;
        }
    }else{
        switch (errno){
            case EACCES:
            case EFAULT:
                fprintf(stderr,"Sem permissao de Acesso\n");
                break;
            case ENOENT:
                fprintf(stderr,"Arquivo inexistente\n");
                break;
            default:
                fprintf(stderr,"Erro desconhecido\n");
            break;
        }
    }
}

void get(unsigned char input[],int tipo){
    FILE* arq ;
    int percentual=100;
    double filelen=0;
    int ret=0;
    int window=0;
    unsigned long buffer[BYTES];
    unsigned char aux[63];
    int tipo_recebido=DEFAULT;
    int size=0;
    int recebeu=0;
    double mem_livre,tamanho=0;
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
                char pwd[63];
                tamanho= atoi(aux);
                filelen=tamanho;
                getcwd(pwd,sizeof(pwd));
                char *bashdf = "df --output=avail --block-size=1 / | tail -n +2 > /tmp/tamanho_gb.txt";
                system(bashdf);
                FILE *memfree = fopen("/tmp/tamanho_gb.txt","r");
                fscanf(memfree,"%lf",&mem_livre);
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
    arq = fopen (input, "w") ;

    while(tipo_recebido!=FIM){
        while(tipo_recebido!=FIM){
            limpa_string(aux,63);
            memset(buffer,0,BYTES);
            errno=0;
            dados.buflen=recv(dados.soquete,buffer,sizeof(unsigned long)*BYTES,0);
            if(errno!=11 && dados.buflen<0){
                fprintf(stderr,"error in reading recv function\n");
                fprintf(stderr,"%s\n",strerror(errno));
                exit(-1);
            }

            if(errno!=11 && buffer[0]==126){
                size=DesmontaBuffer(buffer,aux,&tipo_recebido,&dados.last_seq,&dados.aux);
                if(size==101){
                    window=0;
                    memset(buffer,0,BYTES);
                    input[0]=dados.last_seq;
                    if (dados.sequencia==15)
                        dados.sequencia=0;
                    else
                        dados.sequencia++;
                    tipo=ACK;
                    constroi_buffer(dados.soquete,dados.sequencia,input,tipo,1);
                    break;
                }
                if(size!=FEITO){
                    
                    if(tipo_recebido==NACK){
                        dados.tentativas=0;
                        window=0;
                        memset(buffer,0,BYTES);
                        if(dados.last_seq==15)
                            input[0]=0;
                        else
                            input[0]=dados.last_seq+1;
                      
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
                        filelen-=size;
                        if(floor((filelen/tamanho)*100)!=percentual){
                            percentual--;
                            fprintf(stderr, "\r%.0f%c",floor((filelen/tamanho)*100),37);
                            fflush(stderr);
                        }
                        if(window==4){
                            window=0;
                            memset(buffer,0,BYTES);
                            input[0]=dados.last_seq;
                            if (dados.sequencia==15)
                                dados.sequencia=0;
                            else
                                dados.sequencia++;
                            tipo=ACK;
                            constroi_buffer(dados.soquete,dados.sequencia,input,tipo,1);
                        }
                    }
                } 
            } 
        }
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
            dados.buflen=recv(dados.soquete,buffer,sizeof(unsigned long)*BYTES,0);
            if(errno!=11 && dados.buflen<0){
                fprintf(stderr,"error in reading recv function\n");
                fprintf(stderr,"%s\n",strerror(errno));
                exit(-1);
            }

            if(errno!=11 && buffer[0]==126){
                size=DesmontaBuffer(buffer,aux,&tipo_recebido,&dados.last_seq,&dados.aux);
                if(size!=FEITO){
                    if(tipo_recebido==NACK){
                        dados.tentativas=0;
                        window=0;
                        memset(buffer,0,BYTES);
                        if(dados.last_seq==15)
                            input[0]=0;
                        else
                            input[0]=dados.last_seq+1;
                       
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
                            constroi_buffer(dados.soquete,dados.sequencia,input,tipo,1);
                        }
                    }
                } 
            } 
        }
    }
    dados.tentativas=0;
    tipo_recebido=DEFAULT; 
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
            case PUT:
                put(dir,tipo);
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