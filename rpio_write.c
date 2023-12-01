#include "rpio_write.h"
#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define FILE_MAX_SIZE 0x1000000
#define WRITE_FILE_BUFF_MAX_SIZE 4096

static HANDLE s_rpioWDataLock = NULL;

char FileBaseName[] = "RPIO.txt";

BOOL fileExists(const char *filename) { return (access(filename, F_OK) == 0); }

BOOL openRPIOFile(FILE **pfp)
{
	FILE *fp;

	if (fileExists(FileBaseName))
	{
		if ((fp = fopen(FileBaseName, "a+")) == NULL)
		{
			return FALSE;
		}
	}
	else
	{
		if ((fp = fopen(FileBaseName, "wb")) == NULL)
		{
			return FALSE;
		}
	}
	*pfp = fp;
	return TRUE;
}

void closeRPIOFile(FILE *fp)
{
	if (fp)
	{
		fclose(fp);
	}
}

BOOL writeRPIOData(FILE *fp, char *buffer, int len)
{
	static double lastTime = 0;
	double curTime = GetCurrentTime();
	printf("[writeRPIOData]-datalen = %d\n", len);
	if (curTime - lastTime > 10)
	{
		lastTime = curTime;
		// 获取文件大小
		struct stat file_stat;
		if (stat(FileBaseName, &file_stat) == -1)
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
			sprintf(cmd, "sudo mv %s %s.bak", FileBaseName, FileBaseName);
			printf("cmd: %s\n", cmd);
			system(cmd);
			return FALSE;
		}
	}

	// fseek(fp, 0, SEEK_END); // 定位到文件末尾

	fwrite(buffer, sizeof(char), len, fp);

	return TRUE;
}

void batch_write_file(FILE *fp, char *buffer, int nlen)
{
	if (writeRPIOData(fp, buffer, nlen) == FALSE)
	{
		closeRPIOFile(fp);
		if (openRPIOFile(&fp) == FALSE)
		{
			printf("打开文件失败！\n");
			rw_mutex_unLock(s_rpioWDataLock);
			return;
		}
		writeRPIOData(fp, buffer, nlen);
	}
}

void save_data_fun(void *args)
{
	RPIO_DATA_HANDLE *pHandle = (RPIO_DATA_HANDLE *)args;
	FILE *fp;

	if (openRPIOFile(&fp) == FALSE)
	{
		printf("打开文件失败！\n");
		return;
	}
	if (rw_mutex_wlock(s_rpioWDataLock, 500) != ERR_MUTEX_OK)
	{
		printf("rw_mutex_wlock failed.\n");
		return;
	}
	data_t data;
	int offset = 0;
	char buffer[WRITE_FILE_BUFF_MAX_SIZE + 1] = {0};
	while (1)
	{

		// sem_wait(&pHandle->m_sm);
		if (Linklist_Empty(pHandle->g_link_list))
		{
			usleep(100);
			continue;
		}
		memset(&data, 0, sizeof(data_t));
		while (Linklist_Empty(pHandle->g_link_list) == 0)
		{
			pthread_mutex_lock(&(pHandle->linklist_mutex));
			Linklist_pop(pHandle->g_link_list, &data);
			pthread_mutex_unlock(&(pHandle->linklist_mutex));
#ifdef DEBUG
			// printf("data.nlen=%d \n", data.nlen);
			// printf("data.buffer=\n%s\n", data.buffer);
			// printf("offset=%d\n", offset);
#endif

			static double startTime = 0;
			double endTime = GetCurrentTime();
			if (offset + data.nlen >= WRITE_FILE_BUFF_MAX_SIZE)
			{
				batch_write_file(fp, buffer, offset);
				memset(buffer, 0, WRITE_FILE_BUFF_MAX_SIZE);
				offset = 0;
				startTime = endTime;
			}
			else if (endTime - startTime > 1)
			{
				printf("time to write file.\n");
				if (offset > 0)
				{
					batch_write_file(fp, buffer, offset);
					offset = 0;
					startTime = endTime;
					memset(buffer, 0, WRITE_FILE_BUFF_MAX_SIZE);
				}
			}

			memcpy(buffer + offset, data.buffer, data.nlen);
			offset += data.nlen;
		}
	}
	rw_mutex_unLock(s_rpioWDataLock);
	closeRPIOFile(fp);
}

void rpioWDataLockInit(void) { s_rpioWDataLock = rw_mutex_create(); }

void rpioWDataLockUninit(void) { rw_mutex_destroy(s_rpioWDataLock); }