#include "tcp_server.h"
#include "rpio_data.h"
#include "run_mutex.h"

char *getPeerIP(int nSock)
{
	int nSockLen = sizeof(struct sockaddr);
	struct sockaddr_in addr_in;

	getpeername(nSock, (struct sockaddr *)&addr_in, (socklen_t *)&nSockLen);
	return inet_ntoa((((struct sockaddr_in *)&addr_in)->sin_addr));
}

int create_tcp_server_socket(int *sockfd, int port)
{
	int listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd == -1)
	{
		perror("listenfd failed!\n");
		return 0;
	}
	printf("监听套接字文件描述符：%d\n", listenfd);

	struct sockaddr_in servaddr;
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htons(INADDR_ANY);
	servaddr.sin_port = htons(port);

	/*一般在一个端口释放后需要等一段时间才能重新启用，因此需要借助SO_REUSEADDR来使端口重新启用,
	 解决服务端异常退出之后，再次启动服务端，客户端无法使用同一个端口连接socket的问题*/
	int flags = 1;
	int ret = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &flags, sizeof(flags));
	if (ret < 0)
	{
		perror("setsockopt");
		return succeed_type_failed;
	}

	if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(struct sockaddr_in)) < 0)
	{
		perror("绑定失败！");
		return succeed_type_failed;
	}

	if (listen(listenfd, 5) < 0)
	{
		perror("监听失败！");
		return succeed_type_failed;
	}

	*sockfd = listenfd;
	return succeed_type_succeed;
}

int close_tcp_server_socket(int sock_fd) { close(sock_fd); }

void recv_rpio_fun(void *arg)
{
	RPIO_DATA_HANDLE *pHandle = (RPIO_DATA_HANDLE *)arg;
	int listen_fd = pHandle->listen_fd;

	struct sockaddr_in cliAddr;
	socklen_t len = sizeof(cliAddr);

	// 创建epoll ,int epoll_create(int size); size参数 相当于提供给内核一个提示，当前需要监听的fd个数
	int epoll_fd = epoll_create(MAX_LISTEN_SOCKET);
	struct epoll_event eve;
	eve.data.fd = listen_fd;
	eve.events = EPOLLIN;

	// 注册事件
	int ret = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &eve);
	if (ret == -1)
	{
		printf("epoll注册失败\n");
		close(listen_fd);
		return;
	}
	pHandle->epoll_fd = epoll_fd;
	printf("开始监听！\n");
	struct epoll_event events[MAX_LISTEN_SOCKET];

	rpioDataLockInit();
	while (1)
	{
		int ret;
		int eventCount = epoll_wait(epoll_fd, events, MAX_LISTEN_SOCKET, 500);
		if (eventCount == -1)
		{
			printf("select 出错！\n");
			break;
		}
		else if (eventCount == 0)
		{
			continue;
		}
		// printf("监听到事件数量：%d\n", eventCount);
		for (int i = 0; i < eventCount; i++)
		{
			// 如果是服务器fd并且是读事件，则接收连接
			if (events[i].data.fd == listen_fd)
			{
				// 是否读事件
				if (events[i].events & EPOLLIN)
				{
					int clisock = accept(listen_fd, (struct sockaddr *)&cliAddr, &len);
					if (clisock == -1)
					{
						printf("接收客户端错误\n");
						continue;
					}
					else
					{
						printf("接收到客户端连接\n");
					}

					eve.data.fd = clisock;
					eve.events = EPOLLIN | EPOLLET;
					epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clisock, &eve);
				}
				else
				{
					printf("服务器其他事件\n");
				}
			}
			else
			{
				// 对非服务器socket进行处理
				if (events[i].events & EPOLLIN)
				{
					char buf[1024] = {0};
					size_t len = recv(events[i].data.fd, buf, 1024, 0);
					if (len < 0)
					{
						switch (errno)
						{
						case EAGAIN: // 说明暂时已经没有数据了，要等通知
							break;
						case EINTR: // 被终断
							printf("recv EINTR... \n");
							ret = recv(events[i].data.fd, buf, 1024, 0);
							break;
						default:
							printf("the client is closed, fd:%d\n", events[i].data.fd);
							epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, &eve);
							close(events[i].data.fd);
							break;
						}
					}
					else if (len == 0)
					{
						printf("客户端已经断开连接！\n");
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
						close(events[i].data.fd);
						break;
					}
					else
					{
#ifdef DEBUG
						printf("接收到客户端%d数据：%s\n", events[i].data.fd, buf);
#endif
						writeRPIOData(events[i].data.fd, buf, len);
					}

					// char send_buff[1024];
					// sprintf(send_buff, "hello, i am server, i got your message.");
					// send(events[i].data.fd, send_buff, strlen(send_buff) + 1, 0);
				}
				else if (events[i].events & EPOLLOUT)
				{
					printf("客户端 EPOLLOUT\n");
				}
				else
				{
					printf("客户端其他事件\n");
				}
			}
		}
	}
	rpioDataLockUninit();
	close(epoll_fd);
}