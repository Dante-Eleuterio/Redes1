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
unsigned char *sendbuff;
unsigned short checksum(unsigned short* buff, int _16bitword)
{
  unsigned long sum;
  for(sum=0;_16bitword>0;_16bitword--)
  sum+=htons(*(buff)++);
  sum = ((sum >> 16) + (sum & 0xFFFF));
  sum += (sum>>16);
  return (unsigned short)(~sum);
}
 
int ConexaoRawSocket(char *device)
{
  int soquete;
  struct ifreq ir;
  struct sockaddr_ll endereco;
  struct packet_mreq mr;
  soquete = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));  	/*cria socket*/
  if (soquete == -1) {
    printf("Erro no Socket\n");
    exit(-1);
  }

  memset(&ir, 0, sizeof(struct ifreq));  	/*dispositivo eth0*/
  memcpy(ir.ifr_name, device, sizeof(device));
  if (ioctl(soquete, SIOCGIFINDEX, &ir) == -1) {
    printf("Erro no ioctl:%s\n",strerror(errno));
    exit(-1);
  }

  memset(&endereco, 0, sizeof(endereco)); 	/*IP do dispositivo*/
  endereco.sll_family = AF_PACKET;
  endereco.sll_protocol = htons(ETH_P_ALL);
  endereco.sll_ifindex = ir.ifr_ifindex;
  if (bind(soquete, (struct sockaddr *)&endereco, sizeof(endereco)) == -1) {
    printf("Erro no bind\n");
    exit(-1);
  }
  printf("%d\n",endereco.sll_ifindex);

  struct ifreq ifreq_c;
  memset(&ifreq_c,0,sizeof(ifreq_c));
  strncpy(ifreq_c.ifr_name,device,IFNAMSIZ-1);//giving name of Interface
  
  if((ioctl(soquete,SIOCGIFHWADDR,&ifreq_c))<0) //getting MAC Address
    printf("error in SIOCGIFHWADDR ioctl reading");
  

  /*ETHERNET HEADER*/
  sendbuff= (unsigned char*)malloc(64);
  memset(sendbuff,0,64);
  struct ethhdr *eth = (struct ethhdr *)(sendbuff);

  
  /* filling destination mac. DESTMAC0 to DESTMAC5 are macro having octets of mac address. */
  eth->h_dest[0] = 0x70;
  eth->h_dest[1] = 0x85;
  eth->h_dest[2] = 0xc2;
  eth->h_dest[3] = 0x08;
  eth->h_dest[4] = 0x88;
  eth->h_dest[5] = 0xb7;
  
  eth->h_source[0] = (unsigned char)(ir.ifr_hwaddr.sa_data[0]);
  eth->h_source[1] = (unsigned char)(ir.ifr_hwaddr.sa_data[1]);
  eth->h_source[2] = (unsigned char)(ir.ifr_hwaddr.sa_data[2]);
  eth->h_source[3] = (unsigned char)(ir.ifr_hwaddr.sa_data[3]);
  eth->h_source[4] = (unsigned char)(ir.ifr_hwaddr.sa_data[4]);
  eth->h_source[5] = (unsigned char)(ir.ifr_hwaddr.sa_data[5]);


  eth->h_proto = htons(ETH_P_IP); //means next header will be IP header
 
  /* end of ethernet header */
  int total_len=sizeof(struct ethhdr);
  /*IP HEADER*/
  struct iphdr *iph = (struct iphdr*)(sendbuff + sizeof(struct ethhdr));
  iph->ihl = 5;
  for(int i=0;i<15;i++)
    printf("i:%d data: %d\n",i, sendbuff[i]);
  printf("--------------\n");
  iph->version = 4;
  for(int i=0;i<16;i++)
    printf("i:%d data: %.2X\n",i, sendbuff[i]);
  iph->tos = 16;
  iph->id = htons(10201);
  iph->ttl = 64;
  iph->protocol = 17;
  iph->saddr = inet_addr(inet_ntoa((((struct sockaddr_in *)&(ir.ifr_addr))->sin_addr)));
  iph->daddr = inet_addr("127.0.1.1"); // put destination IP address
 
  total_len += sizeof(struct iphdr);
  /*end of ip header*/

  /*UDP header*/
  struct udphdr *uh = (struct udphdr *)(sendbuff + sizeof(struct iphdr) + sizeof(struct ethhdr));
  uh->source = htons(23451);
  uh->dest = htons(23452);
  uh->check = 0;

  total_len+= sizeof(struct udphdr);
  /*End of udp header*/

  sendbuff[total_len++] = 0xBB;
  sendbuff[total_len++] = 0xBB;
  sendbuff[total_len++] = 0xBB;
  sendbuff[total_len++] = 0xBB;
  sendbuff[total_len++] = 0xBB;
  
  uh->len = htons((total_len - sizeof(struct iphdr) - sizeof(struct ethhdr))); //UDP length field
  printf("uh%d\n",uh->len);
  iph->tot_len = htons(total_len - sizeof(struct ethhdr));//IP length field
  iph->check = checksum((unsigned short*)(sendbuff + sizeof(struct ethhdr)), (sizeof(struct iphdr)/2));
  
  for(int i=0;i<total_len;i++)
    printf("%.2X",sendbuff[i]);

  // memset(&mr, 0, sizeof(mr));          /*Modo Promiscuo*/
  // mr.mr_ifindex = ir.ifr_ifindex;
  // mr.mr_type = PACKET_MR_PROMISC;
  // if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)	{
  //   printf("Erro ao fazer setsockopt\n");
  //   exit(-1);
  // }

  
  return soquete;
}

int main(int argc, char const *argv[])
{
  int send_len= ConexaoRawSocket("lo");
  if(send_len<0)
  {
    printf("error in sending....sendlen=%d....errno=%d\n",send_len,errno);
    return -1;
  }
  printf("\n%ld\n",sendto(send_len,sendbuff,64,0,NULL,sizeof(struct sockaddr_ll)));

  return 0;
}