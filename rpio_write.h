#ifndef __RPIO_WRITE_H__
#define __RPIO_WRITE_H__

#include "run_mutex.h"
#include "server.h"
#include "tcp_client.h"

BOOL openRPIOFile(FILE **pfp);

void closeRPIOFile(FILE *fp);

BOOL writeRPIOData(FILE *fp, char *buffer, int len);

void rpioWDataLockInit(void);

void rpioWDataLockUninit(void);

int flush_list_data(RPIO_DATA_HANDLE *pHandle);

void save_data_fun(void *args);
#endif