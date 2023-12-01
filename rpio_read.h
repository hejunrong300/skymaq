#ifndef __RPIO_READ_H__
#define __RPIO_READ_H__
#include "run_mutex.h"
#include "tcp_client.h"

void registercSendCallfunc(sendDataCallbackFunc func);

BOOL openRPIOFile_R(FILE **pfp);

void closeRPIOFile_R(FILE *fp);

BOOL readRPIOData(const char *fileName, char *buffer, int len);

BOOL writeRPIOData(int fd, char *buffer, int len);

int transfer_data(FILE *fp, TCP_CLIENT_HANDLE *pHandle);
#endif