
#include "rpio_write.h"
#include "tcp_server.h"
#include <signal.h>
#include <unistd.h>

RPIO_DATA_HANDLE g_handle;

void ExitProcess(int signo)
{
	printf("ExitProcess...\n");

	pthread_cancel(g_handle.recvThread);
	pthread_cancel(g_handle.saveDataThread);
	sleep(1);
	pthread_join(g_handle.recvThread, (void *)NULL);
	pthread_join(g_handle.saveDataThread, (void *)NULL);

	flush_list_data(&g_handle);
	closeRPIOFile(g_handle.fp);

	if (g_handle.listen_fd > 0)
	{
		close_tcp_server_socket(g_handle.listen_fd);
	}
	if (g_handle.epoll_fd > 0)
	{
		close(g_handle.epoll_fd);
	}
	pthread_mutex_destroy(&(g_handle.linklist_mutex));
	Linklist_free(g_handle.g_link_list);
	rpioWDataLockUninit();
	_exit(1);
}

int main()
{
	signal(SIGTERM, ExitProcess);
	signal(SIGINT, ExitProcess);
	signal(SIGKILL, ExitProcess);

	memset(&g_handle, 0, sizeof(RPIO_DATA_HANDLE));
	int listen_fd = -1;
	int ret = create_tcp_server_socket(&listen_fd, SOCKET_PORT);
	if (ret != succeed_type_succeed)
	{
		return -1;
	}
	// sem_init(&g_handle.m_sm, 0, 0);
	g_handle.listen_fd = listen_fd;
	pthread_mutex_init(&(g_handle.linklist_mutex), NULL);
	rpioWDataLockInit();

	// tcp server
	if (pthread_create(&g_handle.recvThread, NULL, (void *)&recv_rpio_fun, (void *)&g_handle))
	{
		perror("pthread_create error.");
	}

	if (pthread_create(&g_handle.saveDataThread, NULL, (void *)&save_data_fun, (void *)&g_handle))
	{
		perror("pthread_create error.");
	}

	pthread_join(g_handle.recvThread, (void *)NULL);
	pthread_join(g_handle.saveDataThread, (void *)NULL);

	pthread_mutex_destroy(&(g_handle.linklist_mutex));
	close_tcp_server_socket(listen_fd);
	Linklist_free(g_handle.g_link_list);
	// sem_destroy(&g_handle.m_sm);
	rpioWDataLockUninit();
}
