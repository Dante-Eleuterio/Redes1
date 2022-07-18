#include "header.h"

int retorna_tipo(unsigned char buffer[],int *a,int *l,unsigned char dir[]){

    int i=0;
    unsigned char cmd[7];
    unsigned char arg1[52];
    unsigned char arg2[6];
    sscanf(buffer,"%s %s %s",cmd,arg1,arg2);
    printf("%s %s\n",cmd,arg1);
    if(!strncmp(cmd,"ls",3)){
            if(!strncmp("-a",arg1,2))
                *a=1;
            if(!strncmp("-l",arg1,2))
                *l=1;
            if(!strncmp("-a",arg2,2))
                *a=1;
            if(!strncmp("-l",arg2,2))
                *l=1;
        return LSL;
    }
    if(!strncmp("rls",cmd,3)){
            if(!strncmp("-a",arg1,2))
                *a=1;
            if(!strncmp("-l",arg1,2))
                *l=1;
            if(!strncmp("-a",arg2,2))
                *a=1;
            if(!strncmp("-l",arg2,2))
                *l=1;
        return LS;
    }
    if(!strncmp("cd",cmd,3)){
        strncpy(dir,arg1,52);
        return CDL;
    }
    if(!strncmp("rcd",cmd,3)){
        strncpy(dir,arg1,52);
        return CD;
    }
    if(!strncmp("mkdir",cmd,6)){
        strncpy(dir,arg1,52);
        return MKDIRL;
    }
    if(!strncmp("rmkdir",cmd,6)){
        strncpy(dir,arg1,52);
        return MKDIR;
    }
    else
        return 0;
}
//enp7s0f0
void constroi_buffer(int soquete,unsigned char input[],int sequencia,int tipo){
    unsigned char *sendbuff;
    unsigned char dados[63];
    sendbuff= (unsigned char*)malloc(BYTES);
    memset(sendbuff,0,BYTES);
    header *eth = (header *)(sendbuff);
    eth->mi=126;
    eth->tamanho=strlen(input);
    eth->sequencia=sequencia;
    eth->tipo=tipo;
    int total_len=sizeof(header);
    for (int i = total_len; i < BYTES-1; i++)
    {
        sendbuff[i] = input[i-total_len];
    }
        sendbuff[BYTES-1]=111;
    sendto(soquete,sendbuff,BYTES,0,NULL,0);
}
int main(int argc, char const *argv[])
{
    int arg_a=0;
    int arg_l=0;
    int send_len= ConexaoRawSocket("lo");
    unsigned char input[63];
    unsigned char dir[52];
    int sequencia=0;
    int tipo;
    char cwd[PATH_MAX];
    for (int i = 0; i < 63; i++)
        input[i]=0;
    if(send_len<0)
    {
        printf("error in sending....sendlen=%d....errno=%d\n",send_len,errno);
        return -1;
    }
    system("clear");
    while(1){
        printf("Cliente:%s$ ",getcwd(cwd, sizeof(cwd)));
        fgets(input,63,stdin);
        tipo=retorna_tipo(input,&arg_a,&arg_l,dir);
        switch (tipo)
        {
            case CDL:
                chdir(dir);
                break;
            case LSL:
            {
                if(arg_a==0 && arg_l==0)
                    system("ls");
                if(arg_a!=0 && arg_l==0)
                    system("ls -a");
                if(arg_a==0 && arg_l!=0)
                    system("ls -l");
                if(arg_a!=0 && arg_l!=0)
                    system("ls -a -l");
                arg_a=0;
                arg_l=0;
                break;
            }
            case MKDIRL:
                mkdir(dir,0700);
                break;
            case LS:
                break;
            case CD:
                break;
            case MKDIR:
                constroi_buffer(send_len,dir,sequencia,tipo);
                break;
            default:
                constroi_buffer(send_len,input,sequencia,tipo);
                break;
        }
        sequencia++;
        if (sequencia==8)
            sequencia=0;
    }





  return 0;
}