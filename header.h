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

#define BYTES 14
#pragma pack(1)
struct header{
  unsigned int  mi : 8;
  unsigned int  tamanho:6;
  unsigned int  sequencia:4;
  unsigned int  tipo:6;
};
typedef struct header header;