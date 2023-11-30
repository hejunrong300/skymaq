#include "rpio_data.h"
#include "run_mutex.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

sendDataCallbackFunc g_sendCallBack = NULL;

#define FILE_MAX_SIZE 0x4000000
static HANDLE s_rpioDataLock = NULL;

char FileBaseName[] = "RPIO";
char TestFileName[] = "test.txt";

void registercSendCallfunc(sendDataCallbackFunc func) { g_sendCallBack = func; }

BOOL fileExists(const char *filename) { return (access(filename, F_OK) == 0); }

BOOL readRPIOData(const char *fileName, char *buffer, int len)
{
	FILE *fp;

	if ((fp = fopen(fileName, "rb")) == NULL)
	{
		printf("open %s fail\n", fileName);
		return FALSE;
	}

	if (rw_mutex_rlock(s_rpioDataLock, 500) != ERR_MUTEX_OK)
	{
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

	rw_mutex_unLock(s_rpioDataLock);

	fclose(fp);
	return TRUE;
}

BOOL writeRPIOData(int fd, char *buffer, int len)
{
	FILE *fp;

	char fileName[256] = {0};
	sprintf(fileName, "%s_%d.txt", FileBaseName, fd);

	static double lastTime = 0;
	double curTime = GetCurrentTime();
#ifdef DEBUG
	printf("curTime = %lf\n", curTime);
#endif
	if (curTime - lastTime > 10)
	{
		lastTime = curTime;
		// 获取文件大小
		struct stat file_stat;
		if (stat(fileName, &file_stat) == -1)
		{
			printf("无法获取文件大小。\n");
			return FALSE;
		}
#ifdef DEBUG
		printf("size of file = %ld\n", file_stat.st_size);
#endif
		// 判断文件大小
		if (file_stat.st_size > FILE_MAX_SIZE)
		{
			char cmd[256];
			sprintf(cmd, "sudo mv %s %s_%d_1.txt", fileName, FileBaseName, fd);
			system(cmd);
			if ((fp = fopen(fileName, "wb")) == NULL)
			{
				return FALSE;
			}
		}
	}

	if (fileExists(fileName))
	{
		if ((fp = fopen(fileName, "a+")) == NULL)
		{
			return FALSE;
		}
	}
	else
	{
		if ((fp = fopen(fileName, "wb")) == NULL)
		{
			return FALSE;
		}
	}

	if (rw_mutex_wlock(s_rpioDataLock, 500) != ERR_MUTEX_OK)
	{
		printf("rw_mutex_wlock failed.\n");
		return FALSE;
	}

	fseek(fp, 0, SEEK_END); // 定位到文件末尾

	fwrite(buffer, sizeof(char), len, fp);

	rw_mutex_unLock(s_rpioDataLock);

	fclose(fp);

	return TRUE;
}

void rpioDataLockInit(void) { s_rpioDataLock = rw_mutex_create(); }

void rpioDataLockUninit(void) { rw_mutex_destroy(s_rpioDataLock); }

int transfer_data(int sockfd)
{
	FILE *fp;

	int count, times = 0;
	long fileLen;

	if ((fp = fopen(TestFileName, "rb")) == NULL)
	{
		printf("open %s fail\n", TestFileName);
		return -1;
	}

	if (rw_mutex_rlock(s_rpioDataLock, 500) != ERR_MUTEX_OK)
	{
		return -1;
	}

	char read_buff[1024] = {0};

	while (fread(read_buff, sizeof(char), 1024, fp) > 0)
	{
		printf("reading file:%s\n", read_buff);
		g_sendCallBack(sockfd, read_buff, strlen(read_buff) + 1);
	}

	rw_mutex_unLock(s_rpioDataLock);

	fclose(fp);
	return 0;
}