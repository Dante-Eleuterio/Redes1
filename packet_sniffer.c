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
     printf("Erro no ioctl\n");
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
  memset(&mr, 0, sizeof(mr));          /*Modo Promiscuo*/
   mr.mr_ifindex = ir.ifr_ifindex;
   mr.mr_type = PACKET_MR_PROMISC;
   if (setsockopt(soquete, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1)	{
     printf("Erro ao fazer setsockopt\n");
     exit(-1);
   }

  return soquete;
}
#define BYTES 5

int main (char argc, char argv[]){
  int sock_r;
  sock_r = ConexaoRawSocket("lo");

  unsigned char *buffer = (unsigned char *) malloc(BYTES); //to receive data
  memset(buffer,0,BYTES);
  struct sockaddr saddr;
  int saddr_len = sizeof (saddr);
  
  //Receive a network packet and copy in to buffer
  int flag =1;
  int buflen=recvfrom(sock_r,buffer,BYTES,0,&saddr,(socklen_t *)&saddr_len);
  if(buflen<0){
    printf("error in reading recvfrom function\n");
    return -1;
  }
  for (int i = 0; i < BYTES; i++)
  {
     fprintf(stdout,"%.2X ",buffer[i]);
  }
     fprintf(stdout,"\n");
  
  // struct ethhdr *eth = (struct ethhdr *)(buffer);
  // printf("\nEthernet Header\n");
  // printf("\t|-Source Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
  // printf("\t|-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);
  // printf("\t|-Protocol : %d\n",eth->h_proto);

  // unsigned short iphdrlen;
  // struct iphdr *ip = (struct iphdr*)(buffer + sizeof(struct ethhdr));
  // struct sockaddr_in source;
  // struct sockaddr_in dest;
  // memset(&source, 0, sizeof(source));
  // source.sin_addr.s_addr = ip->saddr;
  // memset(&dest, 0, sizeof(dest));
  // dest.sin_addr.s_addr = ip->daddr;
  // fprintf(stdout, "\t|-Version : %d\n",(unsigned int)ip->version);
  // fprintf(stdout , "\t|-Internet Header Length : %d DWORDS or %d Bytes\n",(unsigned int)ip->ihl,((unsigned int)(ip->ihl))*4);
  // fprintf(stdout , "\t|-Type Of Service : %d\n",(unsigned int)ip->tos);
  // fprintf(stdout , "\t|-Total Length : %d Bytes\n",ntohs(ip->tot_len));
  // fprintf(stdout , "\t|-Identification : %d\n",ntohs(ip->id));
  // fprintf(stdout , "\t|-Time To Live : %d\n",(unsigned int)ip->ttl);
  // fprintf(stdout , "\t|-Protocol : %d\n",(unsigned int)ip->protocol);
  // fprintf(stdout , "\t|-Header Checksum : %d\n",ntohs(ip->check));
  // fprintf(stdout , "\t|-Source IP : %s\n", inet_ntoa(source.sin_addr));
  // fprintf(stdout , "\t|-Destination IP : %s\n",inet_ntoa(dest.sin_addr));

  // /* getting actual size of IP header*/
  // iphdrlen = ip->ihl*4;
  // /* getting pointer to udp header*/
  // struct udphdr *udp=(struct udphdr *)(buffer + iphdrlen + sizeof(struct ethhdr));
  // fprintf(stdout , "\t|-Source Port : %d\n" , ntohs(udp->source));
  // fprintf(stdout , "\t|-Destination Port : %d\n" , ntohs(udp->dest));
  // fprintf(stdout , "\t|-UDP Length : %d\n" , ntohs(udp->len));
  // fprintf(stdout , "\t|-UDP Checksum : %d\n" , ntohs(udp->check));
  // unsigned char * data = (buffer + iphdrlen + sizeof(struct ethhdr) + sizeof(struct udphdr));
  // int remaining_data = buflen - (iphdrlen + sizeof(struct ethhdr) + sizeof(struct udphdr));
 
  // for(int i=0;i<remaining_data;i++){
  //   if(i!=0 && i%16==0)
  //     fprintf(stdout,"\n");
  //   fprintf(stdout,"%.2X",data[i]);
  // }
  // fprintf(stdout,"\n");
}

//while(flag){
  //  buflen=recvfrom(soquete,buffer,1,0,&saddr,(socklen_t *)&saddr_len);
  //  if(buflen<0){
  //    printf("error in reading recvfrom function\n");
  //    return -1;
  //  }
  //  if (buffer="0111110")
  //    func();
  //}