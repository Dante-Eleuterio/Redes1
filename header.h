#include "ConexaoRawSocket.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <linux/if.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <errno.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
//TIPOS DE MENSAGENS
  #define OK 1
  #define NACK 2
  #define ACK 3
  #define CD 6
  #define LS 7
  #define MKDIR 8
  #define GET 9
  #define PUT 10 
  #define ERRO 17
  #define DESCRITOR 24
  #define DADOS 32



  //TIPOS LOCAIS
  #define LSL 35
  #define CDL 36
  #define MKDIRL 37

  #define FIM 46
  #define MOSTRA 63
//FIM TIPOS DE MENSAGENS

//ERROS
  #define FEITO 80

//FIM ERROS

#define DEFAULT 99
#define BYTES 67
#pragma pack(1)

//Header padrão das mensagens
struct header{
  unsigned int  mi : 8;
  unsigned int  tamanho:6;
  unsigned int  sequencia:4;
  unsigned int  tipo:6;
};
typedef struct header header;

//Flags de controle do código
struct args{ 
  int last_seq;
  int last_type;
  unsigned char last_data[63];
  int aux;
  int sequencia;
  int tamanho;
  int tentativas;
  int soquete;
  int buflen;
};
typedef struct args args;

