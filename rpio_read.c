#include "rpio_read.h"
#include "run_mutex.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

sendDataCallbackFunc g_sendCallBack = NULL;

char TestFileName[] = "test.txt";

void registercSendCallfunc(sendDataCallbackFunc func) { g_sendCallBack = func; }

BOOL openRPIOFile_R(FILE **pfp)
{
	printf("openRPIOFile_R...\n");
	FILE *fp;

	int count, times = 0;
	long fileLen;

	if ((fp = fopen(TestFileName, "rb")) == NULL)
	{
		printf("open %s fail\n", TestFileName);
		return FALSE;
	}
	*pfp = fp;
	return TRUE;
}

void closeRPIOFile_R(FILE *fp)
{
	if (fp)
	{
		fclose(fp);
	}
}

BOOL readRPIOData(const char *fileName, char *buffer, int len)
{
	FILE *fp;

	if ((fp = fopen(fileName, "rb")) == NULL)
	{
		printf("open %s fail\n", fileName);
		return FALSE;
	}

	char read_buff[64] = {0};
	int offset = 0;
	while (fread(read_buff, sizeof(char), 64, fp) == 64)
	{
		if (offset > 1024)
		{
			break;
		}

		printf("reading file:%s\n", read_buff);
		memcpy(buffer + offset, read_buff, 64);
		offset += 64;
	}

	fclose(fp);
	return TRUE;
}

int transfer_data(FILE *fp, TCP_CLIENT_HANDLE *pHandle)
{
	fseek(fp, 0, SEEK_SET);
	char read_buff[1024] = {0};

	// while (fread(read_buff, sizeof(char), 1024, fp) != EOF)
	while (fread(read_buff, sizeof(char), 1024, fp) > 0)
	{
		printf("reading file:\n%s\n", read_buff);
		if (g_sendCallBack)
		{
			g_sendCallBack(pHandle, read_buff, strlen(read_buff));
		}
		memset(read_buff, 0, 1024);
		usleep(200);
	}

	return 0;
}