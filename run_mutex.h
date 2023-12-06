#ifndef __RUN_MUTEX_H__
#define __RUN_MUTEX_H__

#include <pthread.h>

enum ERR_CODE_PUB_MODULE_ENUM
{
	ERR_MUTEX_OK,
	ERR_MUTEX_TIMEOUT,
	ERR_INVALID_ARGS
};

#define TRUE 1
#define FALSE 0

#define MAX_WAIT_INTERVAL 5	   // the maximum wait time in seconds.
#define LEN_RUN_THREAD_NAME 16 // the maximum length of the run thread name

typedef int BOOL;
typedef unsigned int DWORD;
typedef void *HANDLE;

typedef struct _RW_MUTEX // internal structure for rw lock
{
	pthread_rwlock_t hrwLock;
} RW_MUTEX;

double GetCurrentTime(void);

HANDLE rw_mutex_create();

int rw_mutex_rlock(HANDLE hm, DWORD dwWaitTimeout);

int rw_mutex_wlock(HANDLE hm, DWORD dwWaitTimeout);

int rw_mutex_unLock(HANDLE hm);

void rw_mutex_destroy(HANDLE hm);

void Init(void);

#endif