//gcc -o server -ggdb -Wall server.c  //--std=gnu99 -lm -lrt
#include <unistd.h>
#include <strings.h>
#include <sys/socket.h>
#include <arpa/inet.h>	//inet_hton socklen_t
#include <netinet/tcp.h> // SOL_TCP
#include <netinet/in.h> // INET_ADDRLEN

#include <stdio.h>
#include <errno.h>
#include <string.h>


#define BACK_LOG 100
#define SERV_PORT 6060
#define SERV_ADDR "10.10.42.22"

#define BUF_SIZE 100
#define EXP_SIZE 23

int main(int argc, char *argv[])
{
	int servfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (servfd < 0)
		return -1;
	struct sockaddr_in servaddr;
	socklen_t addrlen = sizeof(servaddr);
	bzero(&servaddr, sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(SERV_PORT);
	int res = 0;

	if ((res = inet_pton(AF_INET, SERV_ADDR, (struct sockaddr_in*)&servaddr.sin_addr)) == -1)
		return -1;

	if ((res = bind(servfd, (struct sockaddr*)&servaddr, addrlen)) < 0)
		return -1;
	/*for TCP FAST OPEN*/
	/*
	int qlen = 5;
	setsockopt(servfd, 6, //SOL_TCP
			23, //TCP_FASTOPEN
			&qlen, sizeof(qlen));
			*/
	
	if (listen(servfd, BACK_LOG) == -1)
		return -1;

	struct sockaddr_in cliaddr;
	bzero(&cliaddr, addrlen);
	socklen_t cli_addrlen = addrlen;
	int connfd = -1; 

	char recv_buf[BUF_SIZE] = {0};
	int recv_buf_len = sizeof(recv_buf)/sizeof(char);
	char send_buf[BUF_SIZE] = {0};
	int send_buf_len = sizeof(send_buf)/sizeof(char);
	char tmp[INET_ADDRSTRLEN] = {0};

	while((connfd = accept(servfd, (struct sockaddr*)&cliaddr, 
					&cli_addrlen)) > 0) {
		inet_ntop(AF_INET, (struct sockaddr*)&cliaddr.sin_addr, 
				tmp, sizeof(tmp));
		printf("Connection from connfd %s:%d\n",
				tmp, ntohs(cliaddr.sin_port));
				
		int size = 0, i = 0;
		// why not block
		while((i = recv(connfd, recv_buf, recv_buf_len, 0)) > 0) {
			size += i;
			printf("Recv %d bytes data from client.\n", i);
			printf("%s\n", recv_buf);
			if (send(connfd, recv_buf, strlen(recv_buf), 0) < 0) {
				printf("Send Error %d\n", errno);
			}
			bzero(recv_buf, recv_buf_len);
			if (size >= EXP_SIZE)
				break;
		}
		/*
		recv_buf[size] = 0;
		i = 0;
		while(recv_buf[i] != 0) {
			send_buf[i] = recv_buf[i];
			i++;
		}
		if ((res = send(connfd, send_buf, size, 0)) < 0) {
			return -1;
		}
		*/
		printf("Send %d bytes data.\n", size);
	}
	
	return 0;
}
