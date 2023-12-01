#ifndef __TCP_CLIENT_SOCKET_H__
#define __TCP_CLIENT_SOCKET_H__
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SERVER_IP "192.168.18.128"
#define PORT 8888 // 端口号

#define MAX_RECV_BUFF_SIZE 1024

typedef struct _TCP_CLIENT_HANDLE
{
	int sockfd;
	char serIp[64];
} TCP_CLIENT_HANDLE;

typedef void (*sendDataCallbackFunc)(TCP_CLIENT_HANDLE *pHandle, char *buffer, int len);

int tcp_client_sock_init();
int sock_conncet(int sock_fd, const char *serip);
int tcp_client_send_data(TCP_CLIENT_HANDLE *pHandle, char *buff, int datalen);
int tcp_client_recv_data(int sock_fd, char *buff, int datalen);
int tcp_client_sock_close(int sock_fd);
int tcp_socket_restart(TCP_CLIENT_HANDLE *pHandle);

#endif