#include "header.h"
#include <stdio.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>
int last_seq;


void DesmontaBuffer(unsigned char buffer[]){
  header *hd = (header *)(buffer);
  if(last_seq!=hd->sequencia){
      last_seq=hd->sequencia;
      printf("HEADER");
      printf("\t|mi :%d\n ",hd->mi);
      printf("\t|tamanho :%d\n ",hd->tamanho);
      printf("\t|sequencia :%d\n ",hd->sequencia);
      printf("\t|tipo :%d\n",hd->tipo);
      unsigned char * data = (buffer + sizeof(header));
      for (int i = 0; i < hd->tamanho; i++)
        printf("i:%d data:%c\n",i,data[i]);
      printf("paridade: %d\n",buffer[BYTES-1]);
    }
    
}


int main(){
  int sock_r;
  sock_r = ConexaoRawSocket("lo");
  unsigned char *buffer = (unsigned char *) malloc(BYTES); //to receive data
  memset(buffer,0,BYTES);
  int buflen;
  last_seq=-1;
  while(1)
  {  
    buflen=recvfrom(sock_r,buffer,BYTES,0,NULL,0);
    if(buflen<0){
      printf("error in reading recvfrom function\n");
      return -1;
    }
  if(buffer[0]==126)
    DesmontaBuffer(buffer);
  }
  return 0;
}
// int main (char argc, char argv[]){
//   int sock_r;
//   sock_r = ConexaoRawSocket("enp7s0f0");

//   unsigned char *buffer = (unsigned char *) malloc(BYTES); //to receive data
//   memset(buffer,0,BYTES);
//   struct sockaddr saddr;
//   int saddr_len = sizeof (saddr);
  
//   //Receive a network packet and copy in to buffer
//   int flag =1;
//   int buflen=recvfrom(sock_r,buffer,BYTES,0,&saddr,(socklen_t *)&saddr_len);
//   if(buflen<0){
//     printf("error in reading recvfrom function\n");
//     return -1;
//   }
//   for (int i = 0; i < BYTES; i++)
//   {
//      fprintf(stdout,"%d ",buffer[i]);
//   }
//      fprintf(stdout,"\n");
  
   //struct ethh dr *eth = (struct ethhdr *)(buffer);
//   printf("\nEthernet Header\n");
//   printf("\t|-Source Address : %d-%d-%d-%d-%d-%d\n",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
//   printf("\t|-Destination Address : %d-%d-%d-%d-%d-%d\n",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);
//   printf("\t|-Protocol : %d\n",eth->h_proto);

//   unsigned short iphdrlen;
//   struct iphdr *ip = (struct iphdr*)(buffer + sizeof(struct ethhdr));
//   struct sockaddr_in source;
//   struct sockaddr_in dest;
//   memset(&source, 0, sizeof(source));
//   source.sin_addr.s_addr = ip->saddr;
//   memset(&dest, 0, sizeof(dest));
//   dest.sin_addr.s_addr = ip->daddr;
//   fprintf(stdout, "\t|-Version : %d\n",(unsigned int)ip->version);
//   fprintf(stdout , "\t|-Internet Header Length : %d DWORDS or %d Bytes\n",(unsigned int)ip->ihl,((unsigned int)(ip->ihl))*4);
//   fprintf(stdout , "\t|-Type Of Service : %d\n",(unsigned int)ip->tos);
//   fprintf(stdout , "\t|-Total Length : %d Bytes\n",ntohs(ip->tot_len));
//   fprintf(stdout , "\t|-Identification : %d\n",ntohs(ip->id));
//   fprintf(stdout , "\t|-Time To Live : %d\n",(unsigned int)ip->ttl);
//   fprintf(stdout , "\t|-Protocol : %d\n",(unsigned int)ip->protocol);
//   fprintf(stdout , "\t|-Header Checksum : %d\n",ntohs(ip->check));
//   fprintf(stdout , "\t|-Source IP : %s\n", inet_ntoa(source.sin_addr));
//   fprintf(stdout , "\t|-Destination IP : %s\n",inet_ntoa(dest.sin_addr));

//   /* getting actual size of IP header*/
//   iphdrlen = ip->ihl*4;
//   /* getting pointer to udp header*/
//   struct udphdr *udp=(struct udphdr *)(buffer + iphdrlen + sizeof(struct ethhdr));
//   fprintf(stdout , "\t|-Source Port : %d\n" , ntohs(udp->source));
//   fprintf(stdout , "\t|-Destination Port : %d\n" , ntohs(udp->dest));
//   fprintf(stdout , "\t|-UDP Length : %d\n" , ntohs(udp->len));
//   fprintf(stdout , "\t|-UDP Checksum : %d\n" , ntohs(udp->check));
//   unsigned char * data = (buffer + iphdrlen + sizeof(struct ethhdr) + sizeof(struct udphdr));
//   int remaining_data = buflen - (iphdrlen + sizeof(struct ethhdr) + sizeof(struct udphdr));
 
//   for(int i=0;i<remaining_data;i++){
//     if(i!=0 && i%16==0)
//       fprintf(stdout,"\n");
//     fprintf(stdout,"%d",data[i]);
//   }
//   fprintf(stdout,"\n");
// }

//while(flag){
  //  buflen=recvfrom(soquete,buffer,1,0,&saddr,(socklen_t *)&saddr_len);
  //  if(buflen<0){
  //    printf("error in reading recvfrom function\n");
  //    return -1;
  //  }
  //  if (buffer="0111110")
  //    func();
  //}