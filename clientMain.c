
#include "rpio_read.h"
#include "tcp_client.h"
#
#include <signal.h>

TCP_CLIENT_HANDLE g_handle;
static HANDLE s_rpioDataLock = NULL;

void ExitProcess(int signo)
{
	printf("ExitProcess...\n");
	if (g_handle.sockfd > 0)
	{
		tcp_client_sock_close(g_handle.sockfd);
	}
	pthread_rwlock_destroy(&g_handle.rwlock);
	rpioDataLockUninit();
	_exit(1);
}

void rpioDataLockInit(void) { s_rpioDataLock = rw_mutex_create(); }

void rpioDataLockUninit(void) { rw_mutex_destroy(s_rpioDataLock); }

// 定义回调函数
void callbackFunction(int sock_fd, char *buffer, int len) { tcp_client_send_data(sock_fd, buffer, len); }

void send_fun(void *args)
{
	TCP_CLIENT_HANDLE *pHandle = (TCP_CLIENT_HANDLE *)args;
	FILE *fp;
	if (rw_mutex_rlock(s_rpioDataLock, 500) != ERR_MUTEX_OK)
	{
		return;
	}
	if (openRPIOFile_R(&fp) == FALSE)
	{
		printf("打开文件失败！\n");
		rw_mutex_unLock(s_rpioDataLock);
		return;
	}

	while (1)
	{
		transfer_data(fp, pHandle);
		usleep(500);
	}
	closeRPIOFile_R(fp);
	rw_mutex_unLock(s_rpioDataLock);
}

int main(int argc, char *argv[])
{
	signal(SIGTERM, ExitProcess);
	signal(SIGINT, ExitProcess);
	signal(SIGKILL, ExitProcess);

	if (argc < 2)
	{
		printf("please input server ip for parameter!\n");
		return -1;
	}

	char serIp[64] = {0};
	sprintf(serIp, "%s", argv[1]);
	memset(&g_handle, 0, sizeof(TCP_CLIENT_HANDLE));

	int fd = tcp_client_sock_init();
	if (fd < 0)
	{
		printf("create client socket failure!\n");
		return -1;
	}
	int ret = sock_conncet(fd, serIp);
	if (ret < 0)
	{
		printf("client connect to server failure!\n");
		return -1;
	}
	g_handle.sockfd = fd;
	rpioDataLockInit();

	registercSendCallfunc(callbackFunction);

	pthread_t sendThread; // 数据包发送线程

	if (pthread_create(&sendThread, NULL, (void *)&send_fun, (void *)&g_handle))
	{
		perror("pthread_create error.");
	}
	pthread_join(sendThread, (void *)NULL);

	tcp_client_sock_close(fd);
	rpioDataLockUninit();
	return 0;
}