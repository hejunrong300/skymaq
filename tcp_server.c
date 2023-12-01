#include "tcp_server.h"
#include "rpio_write.h"
#include "run_mutex.h"

#define REVC_BUFFER_MAX_SIZE 4096

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
	printf("listenfd=%d\n", listenfd);

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
		perror("bind fail");
		return succeed_type_failed;
	}

	if (listen(listenfd, 5) < 0)
	{
		perror("listen fail");
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
		printf("epoll_ctl failure\n");
		close(listen_fd);
		return;
	}
	pHandle->epoll_fd = epoll_fd;
	printf("start listen\n");
	struct epoll_event events[MAX_LISTEN_SOCKET];

	if (NULL == pHandle->g_link_list)
	{
		pHandle->g_link_list = Create_Linklist();
	}

	while (1)
	{
		int ret;
		int eventCount = epoll_wait(epoll_fd, events, MAX_LISTEN_SOCKET, 500);
		if (eventCount == -1)
		{
			printf("select error！\n");
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
						printf("Receiving client error\n");
						continue;
					}
					else
					{
						printf("Client connection received\n");
					}

					eve.data.fd = clisock;
					eve.events = EPOLLIN | EPOLLET;
					epoll_ctl(epoll_fd, EPOLL_CTL_ADD, clisock, &eve);
				}
				else
				{
					printf("Server other events\n");
				}
			}
			else
			{
				// 对非服务器socket进行处理
				if (events[i].events & EPOLLIN)
				{
					char buf[REVC_BUFFER_MAX_SIZE] = {0};
					size_t len = recv(events[i].data.fd, buf, REVC_BUFFER_MAX_SIZE, 0);
					if (len < 0)
					{
						switch (errno)
						{
						case EAGAIN: // 说明暂时已经没有数据了，要等通知
							break;
						case EINTR: // 被终断
							printf("recv EINTR... \n");
							ret = recv(events[i].data.fd, buf, REVC_BUFFER_MAX_SIZE, 0);
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
						printf("The client is disconnected!\n");
						epoll_ctl(epoll_fd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
						close(events[i].data.fd);
						break;
					}
					else
					{
						// printf("client_fd = %d\n", events[i].data.fd);
						if (len < REVC_BUFFER_MAX_SIZE)
						{
							data_t data;
							memset(&data, 0, sizeof(data_t));
							memcpy(data.buffer, buf, len);
							data.nlen = len;
							pthread_mutex_lock(&(pHandle->linklist_mutex));
							Linklist_Insert(pHandle->g_link_list, data);
							pthread_mutex_unlock(&(pHandle->linklist_mutex));
							// sem_post(&pHandle->m_sm);
						}
						else
						{
							close(events[i].data.fd);
						}
					}

					// char send_buff[1024];
					// sprintf(send_buff, "hello, i am server, i got your message.");
					// send(events[i].data.fd, send_buff, strlen(send_buff) + 1, 0);
				}
				else if (events[i].events & EPOLLOUT)
				{
					printf("client EPOLLOUT\n");
				}
				else
				{
					printf("Other events on the client.\n");
				}
			}
		}
	}

	close(epoll_fd);
}
