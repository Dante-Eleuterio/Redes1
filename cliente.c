#include "header.h"
void _ls(int op_a,int op_l)
{
	//Here we will list the directory
	struct dirent *d;
	DIR *dh = opendir(".");
	if (!dh)
	{
		if (errno = ENOENT)
		{
			//If the directory is not found
			perror("Directory doesn't exist");
		}
		else
		{
			//If the directory is not readable then throw error and exit
			perror("Unable to read directory");
		}
		exit(EXIT_FAILURE);
	}
	//While the next entry is not readable we will print directory files
	while ((d = readdir(dh)) != NULL)
	{
		//If hidden files are found we continue
		if (!op_a && d->d_name[0] == '.')
			continue;
		printf("%s  ", d->d_name);
		if(op_l) printf("\n");
	}
	if(!op_l)
	printf("\n");
    closedir(dh);
}

void constroi_buffer(int soquete,unsigned char input[],int sequencia,int tipo){
    unsigned char *sendbuff;
    unsigned char dados[64];
    sendbuff= (unsigned char*)malloc(BYTES);
    memset(sendbuff,0,BYTES);
    header *eth = (header *)(sendbuff);
    eth->mi=126;
    eth->tamanho=strlen(input)-1;
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
int retorna_tipo(unsigned char buffer[],int a,int l,unsigned char dir[]){

    int i=0;
    if(!strncmp("ls",buffer,2)){
        i=3;
        while (buffer[i]!='\0')
        {
            if(buffer[i]=='-'){
                i++;
                if(buffer[i]=='a')
                    a=1;
                else
                if(buffer[i]=='l')
                    l=1;
            }
            i++;
        }
        return LSL;
    }
    if(!strncmp("cd",buffer,2)){
        i=3;
        while(buffer[i]!='\0' && ){

        }
    }
    else
        return 0;
}
//enp7s0f0
int main(int argc, char const *argv[])
{
    int a=0;
    int l=0;
    int send_len= ConexaoRawSocket("lo");
    unsigned char input[64];
    unsigned char dir[60];
    int sequencia=0;
    int tipo;
    char cwd[PATH_MAX];
    for (int i = 0; i < 64; i++)
        input[i]=0;
    if(send_len<0)
    {
        printf("error in sending....sendlen=%d....errno=%d\n",send_len,errno);
        return -1;
    }
    system("clear");
    while(1){
        printf("Cliente:%s$ ",getcwd(cwd, sizeof(cwd)));
        fgets(input,64,stdin);
        tipo=retorna_tipo(input,a,l,dir);
        switch (tipo)
        {
            case CDL:
                Change_Directory()
                break;
            case LSL:
                _ls(a,l);
                break;
            case MKDIRL:
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