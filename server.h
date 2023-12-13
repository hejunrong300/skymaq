#ifndef __SERVER_H__
#define __SERVER_H__

#include "linklist.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define WRITE_FILE_BUFF_MAX_SIZE 4096
typedef struct _RPIO_DATA_HANDLE
{
	int listen_fd;
	int epoll_fd;
	FILE *fp;
	pthread_mutex_t linklist_mutex;
	LinkList *g_link_list;
	sem_t m_sm;
	int offset;
	pthread_t recvThread;
	pthread_t saveDataThread;
	char buffer[WRITE_FILE_BUFF_MAX_SIZE];
} RPIO_DATA_HANDLE;

#endif
