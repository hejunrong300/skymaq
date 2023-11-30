#ifndef __RPIO_DATA_H__
#define __RPIO_DATA_H__
#include "run_mutex.h"
#include "tcp_client.h"

void registercSendCallfunc(sendDataCallbackFunc func);

void rpioDataLockInit(void);

void rpioDataLockUninit(void);

BOOL readRPIOData(const char *fileName, char *buffer, int len);

BOOL writeRPIOData(int fd, char *buffer, int len);

int transfer_data(int sockfd);
#endif