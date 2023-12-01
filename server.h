#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <semaphore.h>
#include "linklist.h"

typedef struct _RPIO_DATA_HANDLE
{
	int listen_fd;
	int epoll_fd;
	FILE *fp;
	pthread_mutex_t linklist_mutex;
	LinkList *g_link_list;
    sem_t       m_sm;
} RPIO_DATA_HANDLE;

#endif

