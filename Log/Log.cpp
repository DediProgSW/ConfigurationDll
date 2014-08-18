// Log.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <time.h>
#include <stdio.h>
#include <direct.h>
#include <assert.h>
#include <stdarg.h>
#include "log.h"
#include "../common/assist.h"

static struct {
        enum log_level  level;
        HANDLE          mutex;    
        FILE            *file;
} log;

int log_init_c(const char *name)
{
	char buff[MAX_PATH];
	struct tm *time_info;
	time_t timeparam;

	time(&timeparam);

	time_info = localtime(&timeparam);

	_mkdir("C:\\DediLog");

	sprintf(buff, "C:\\DediLog\\%s-%d-%d-%d.log",
			name,
                        time_info->tm_year + 1990, time_info->tm_mon + 1, time_info->tm_mday);

	log.file = fopen(buff, "a+");

	if (!log.file)
		return -1;

        log.mutex = CreateMutex(NULL, FALSE, NULL);
        log.level = LOG_INFO;

	return 0;	
}

int log_set_level_c(enum log_level level)
{
        log.level = level;

        return 0;
}

int log_level_add_ascii(enum log_level level, const char *format, ...)
{
	char tmp[1024];
	struct tm *time_info;
	time_t timeparam;
	va_list vl;

	assert(log.file);
	assert(format);

        if (level > log.level)
                return 0;

	time(&timeparam);
	time_info = localtime(&timeparam);

	sprintf(tmp, "%d:%d:%d-->",
		time_info->tm_hour,
		time_info->tm_min,
		time_info->tm_sec
		);

        WaitForSingleObject(log.mutex, 20000);        /* timeout 10s */
	if (strlen(tmp) != fwrite(tmp, sizeof(char), strlen(tmp), log.file))
		return -1;

	va_start(vl, format);
	vsnprintf(tmp, ARRAY_SIZE(tmp), format, vl);

	if (strlen(tmp) != fwrite(tmp, sizeof(char), strlen(tmp), log.file))
		return -1;

	if (strlen("\r\n") != fwrite("\r\n", sizeof(char), strlen("\r\n"), log.file))
		return -1;

	fflush(log.file);

#ifdef _DEBUG
        printf("%s%s", tmp, "\n");
#endif
        ReleaseMutex(log.mutex);

	return 0;
}

int log_exit_c()
{
	assert(log.file);

	fclose(log.file);
	log.file = NULL;

	return 0;
}