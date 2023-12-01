#include "tcp_client.h"
#include "rpio_read.h"
#include "run_mutex.h"

int tcp_client_sock_init()
{
	int sock = 0;
	struct sockaddr_in serv_addr; // IPV4 地址结构体
	// 创建 TCP 套接字
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket creation error");
		return -1;
	}
	printf("client fd = %d\n", sock);

	return sock;
}

// 连接服务器
int sock_conncet(int sock_fd, const char *serip)
{
	struct sockaddr_in serv_addr; // IPV4 地址结构体

	// 设置服务器地址和端口
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);

	// 将 IPv4 地址从点分十进制转换为二进制
	if (inet_pton(AF_INET, serip, &serv_addr.sin_addr) <= 0)
	{
		printf("Invalid address/ Address not supported \n");
		return -1;
	}

	if (connect(sock_fd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("Connection Failed \n");
		return -1;
	}
}

int tcp_client_send_data(int sock_fd, char *buff, int datalen)
{
	if (sock_fd < 0)
	{
		return -1;
	}
	return send(sock_fd, buff, datalen, 0);
}

int tcp_client_recv_data(int sock_fd, char *buff, int datalen)
{
	if (sock_fd < 0)
	{
		return -1;
	}
	return read(sock_fd, buff, datalen);
}

// 关闭连接
int tcp_client_sock_close(int sock_fd) { close(sock_fd); }
