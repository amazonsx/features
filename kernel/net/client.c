//gcc -o client -ggdb -Wall client.c  --std=gnu11 -lm -lrt
#include <unistd.h>
//error "Never include <bits/socket.h> directly; use <sys/socket.h> instead."
//#include <bits/socket.h> // -> socket_type.h SOCK_STREAM
#include <sys/socket.h> // -> socket_type.h SOCK_STREAM
#include <netinet/in.h> // IPPROTO_SOCK
#include <arpa/inet.h> // inet_pton
#include <strings.h> // bzero
// different gcc lib version use different things
//#include <linux/time.h> // CLOCK_MONOTONIC, struct time_spec

#include <stdlib.h>	//malloc
#include <stdio.h>	//printf, sprintf
#include <time.h>	//
#include <errno.h>	//
#include <string.h>	//atoi, strcat
#include <math.h>	//pow execute gcc with -lm

#define COUNT 100
#define BUF_SIZE 100
#define MAX  1000001
#define MIN  1
#define STEP 500

#define MAXSTR 100

#define SERV_PORT 6060
#define SERV_ADDR "10.10.42.22"

#define DEBUG 0

// retrograde
//#define _POSIX_C_SOURCE  199309L

void tcp_open_init(char *dest) {
	char port[5] = {0};
	sprintf(port, "%d", SERV_PORT);
	strcat(dest, "http://");
	strcat(dest, SERV_ADDR); 
	strcat(dest, ":");
	strcat(dest, port);
}

int tcp_fast_open(int clifd, struct sockaddr_in *servaddr, char* buf, int buflen) {
	//char dest[MAXSTR] = {0};
	char *dest = malloc(MAXSTR);
	tcp_open_init(dest);
	//int data_size = sizeof(dest)/sizeof(char); //wrong way to cal a string
	//len, which is pointed by a pointer. Diff from char array
	int data_size = strlen(dest); //+ 1;
	if ((data_size = sendto(clifd, dest, data_size, 0x20000000, //MSG_FASTOPEN
			(struct sockaddr*)servaddr, sizeof(struct sockaddr_in))) < 0) {
		free(dest);
		return -1;
	}
	int len = 0, i = 0;
	char *start = buf;
	//while((i = recv(clifd, buf, buflen, MSG_DONTWAIT)) != 0) {
	while((i = recv(clifd, buf, buflen, 0)) != 0 && len != 23) {
		//printf("%s\n", buf);
		if (i == -1) {
			if (errno == EAGAIN)
				continue;
			else
				break;
		} 
		//printf("Recv %d bytes.\n", i);
		start = buf;
		buf += i;
		len += i;
		buflen -= i;
		if (len == 23)
			break;
	}
	/*
	while((i = read(clifd, buf + len, 1)) > 0) {
		printf("buf:%s\n", buf);
		len += i;
	}
	*/
	if (i < 0 && errno != EAGAIN)
		return -1;
	start[len] = 0;
	free(dest);
	return len;
}

int tcp_normal_open(int clifd, struct sockaddr_in *servaddr, 
		char* buf, int buflen){

	if (connect(clifd, (struct sockaddr*)&servaddr, 
				sizeof(struct sockaddr_in)) == -1)
	   	return -1;

	char *dest = malloc(MAXSTR); 
	tcp_open_init(dest);
	int data_size = strlen(dest);
	if (send(clifd, dest, data_size, 0) < 0) {
		free(dest);
		return -1;
	}
	int len = 0, i = 0;
	char *start = buf;
	while( (i = recv(clifd, buf, buflen, 0)) > 0 && len != 23) {
		buf += i;
		len += i;
		buf += i;
		buflen -= i;
		if (len == 23)
			break;
	}
	if (i < 0)
		return -1;
	start[len] = 0;
	free(dest);
	return len;
}


int main(int argc, char *argv[])
{
#if DEBUG == 0
	for (int i = 0; i < argc; i ++) {
		printf("The %ith arg:%s\n", i, argv[i]);
	}
#endif
	if (argc == 1) {
		printf("First arg should be the feature code,\n"
				"Second arg should be the run mode!\n");
		return 0;
	}
	// feature number for specific test client
	// for features need a cyclic-test
	int feature_code = atoi(argv[1]);
	int run_mode = atoi(argv[2]);
	//double overhead = 0;

	char buf[BUF_SIZE] = {0};
	unsigned long total_overhead = 0;
	for (int len = 0; len <= COUNT; len += 1) {
		struct timespec start;
		clock_gettime(CLOCK_MONOTONIC, &start);
		int clifd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		struct sockaddr_in servaddr;
		bzero(&servaddr, sizeof(servaddr));
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(SERV_PORT);
		//printf("The value of SERV_PORT is %d\n", htons(SERV_PORT));
		if (inet_pton(AF_INET, SERV_ADDR, (struct sockaddr_in*)&servaddr.sin_addr) != 1)
			return -1;

		int res = 0;
		switch(run_mode) {
			case 0:
				res = tcp_fast_open(clifd, &servaddr, 
						buf, sizeof(buf)/sizeof(char));
				break;
			case 1:
				res = tcp_normal_open(clifd, &servaddr, 
						buf, sizeof(buf)/sizeof(char));
				break;
			default:
				break;
		}
		if (res < 0) 
			printf("%d with %d runs error\n", feature_code, run_mode);
		bzero(buf, sizeof(buf));
		close(clifd);
		struct timespec end;
		clock_gettime(CLOCK_MONOTONIC, &end);
		unsigned long diff = 0;
		if (end.tv_sec > start.tv_sec && end.tv_nsec < start.tv_nsec) {
			diff = (end.tv_sec - start.tv_sec - 1) * pow(10, 9) 
				+ pow(10, 9) + end.tv_nsec - start.tv_nsec;
		} else {
			diff = (end.tv_sec - start.tv_sec) * pow(10, 9) 
				+ end.tv_nsec - start.tv_nsec;
		}
		printf("Overhead: %lu\n", diff);
		total_overhead += diff;
	}
	printf("Total Overhead: %lu\n", total_overhead);
	printf("Average Overhead: %10f\n", ((double)total_overhead)/COUNT);
	
	return 0;
}
