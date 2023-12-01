#ifndef __TCP_SERVER_H__
#define __TCP_SERVER_H__

#include "linklist.h"
#include "server.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define MAX_LISTEN_SOCKET 256
#define SOCKET_PORT 8888

typedef enum succeed_type_type
{
	succeed_type_failed = -1,
	succeed_type_succeed,
} succeed_type;

int create_tcp_server_socket(int *sockfd, int port);

int close_tcp_server_socket(int sock_fd);

void recv_rpio_fun(void *arg);

#endif