#include "run_mutex.h"
#include <malloc.h>
#include <sys/time.h>
#include <unistd.h>

double GetCurrentTime(void)
{

	struct timeval tv;

	gettimeofday(&tv, NULL);

	return (double)tv.tv_sec + (double)tv.tv_usec / (double)1000000;
}

HANDLE rw_mutex_create()
{
	RW_MUTEX *pRW_Mutex = (RW_MUTEX *)malloc(sizeof(RW_MUTEX));
	if (pRW_Mutex == NULL)
	{
		printf("[Mutex_Create] -- out of memory on new a MUTEX.\n");
		return NULL;
	}

	int err;
	err = pthread_rwlock_init(&(pRW_Mutex->hrwLock), NULL);
	if (err != 0)
	{
		printf("lock init error\n");
		return NULL;
	}

	return (HANDLE)pRW_Mutex;
}

#define RW_WAIT_LOCK_INTERVAL 20000
int rw_mutex_rlock(HANDLE hm, DWORD dwWaitTimeout)
{
	RW_MUTEX *pRW_Mutex = (RW_MUTEX *)hm;
	double tLast, tNow;
	double fWaitTimeout, fTimeLastWaited, fTimeWaitedAfterHeartbeat = 0;
	if (hm == NULL)
	{
		return ERR_INVALID_ARGS;
	}
#ifdef DEBUG
	printf("[Mutex_Lock] -- close_tcp_server_socket %d ms at %d\n", (int)dwWaitTimeout, (int)time(NULL));

#endif

	tLast = GetCurrentTime();
	fWaitTimeout = (double)dwWaitTimeout / 1000.0;

	for (;;)
	{
		if (pthread_rwlock_tryrdlock(&(pRW_Mutex->hrwLock)) == 0)
		{
			return ERR_MUTEX_OK;
		}

		// to test timeout.
		tNow = GetCurrentTime();
		fTimeLastWaited = tNow - tLast;
		tLast = tNow;

		// the time may be changed to future or back
		if ((fTimeLastWaited > 10.0 * RW_WAIT_LOCK_INTERVAL / 1000000) || (fTimeLastWaited < 0))
		{
			fTimeLastWaited = 1.0 * RW_WAIT_LOCK_INTERVAL / 10000000;
		}

		// trigger heartbeart
		fTimeWaitedAfterHeartbeat += fTimeLastWaited;
		if (fTimeWaitedAfterHeartbeat >= MAX_WAIT_INTERVAL)
		{
			fTimeWaitedAfterHeartbeat = 0;
		}

		fWaitTimeout -= fTimeLastWaited;
		if (fWaitTimeout <= 0)
		{
			break;
		}
		// continue to try to lock.
		//  sleep a while to try, always use the interval 20 ms
		usleep(RW_WAIT_LOCK_INTERVAL); // unit is us
	}

#ifdef DEBUG
	printf("[Mutex_Lock] -- lock timeouted at %d\n", (int)time(NULL));
#endif

	return ERR_MUTEX_TIMEOUT;
}

int rw_mutex_wlock(HANDLE hm, DWORD dwWaitTimeout)
{
	RW_MUTEX *pRW_Mutex = (RW_MUTEX *)hm;
	double tLast, tNow;
	double fWaitTimeout, fTimeLastWaited, fTimeWaitedAfterHeartbeat = 0; // s

	if (hm == NULL)
	{
		return ERR_INVALID_ARGS;
	}

#ifdef DEBUG
	printf("[Mutex_Lock] -- close_tcp_server_socket %d ms at %d\n", (int)dwWaitTimeout, (int)time(NULL));
#endif

	tLast = GetCurrentTime();
	fWaitTimeout = (double)dwWaitTimeout / 1000.0;

	for (;;)
	{
		if (pthread_rwlock_trywrlock(&pRW_Mutex->hrwLock) == 0)
		{
#ifdef DEBUG
			printf("pthread_rwlock_trywrlock ok!\n");
#endif
			return ERR_MUTEX_OK;
		}

		// to test timeout.
		tNow = GetCurrentTime();
		fTimeLastWaited = tNow - tLast;
		tLast = tNow;

		// the time may be changed to future or back
		if ((fTimeLastWaited > 10.0 * RW_WAIT_LOCK_INTERVAL / 1000000) || (fTimeLastWaited < 0))
		{
			fTimeLastWaited = 1.0 * RW_WAIT_LOCK_INTERVAL / 10000000;
		}

		// trigger heartbeart
		fTimeWaitedAfterHeartbeat += fTimeLastWaited;
		if (fTimeWaitedAfterHeartbeat >= MAX_WAIT_INTERVAL)
		{
			fTimeWaitedAfterHeartbeat = 0;
		}

		fWaitTimeout -= fTimeLastWaited;
		if (fWaitTimeout <= 0)
		{
			break;
		}
		// continue to try to lock.
		//  sleep a while to try, always use the interval 20 ms
		usleep(RW_WAIT_LOCK_INTERVAL); // unit is us
	}

#ifdef DEBUG
	printf("[Mutex_Lock] -- lock timeouted at %d\n", (int)time(NULL));
#endif
	return ERR_MUTEX_TIMEOUT;
}

int rw_mutex_unLock(HANDLE hm)
{
	RW_MUTEX *pRW_Mutex = (RW_MUTEX *)hm;
	pthread_rwlock_unlock(&((RW_MUTEX *)hm)->hrwLock);
}

void rw_mutex_destroy(HANDLE hm)
{
	if (hm != NULL)
	{
		pthread_rwlock_destroy(&((RW_MUTEX *)hm)->hrwLock);
		free(hm);
	}
}