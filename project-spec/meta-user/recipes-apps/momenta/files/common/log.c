/*************************************************************************
	> File Name: log.c
	> Author: XXDK
	> Email: v.manstein@qq.com
	> Created Time: Fri 01 Nov 2019 10:19:40 AM CST
 ************************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "log.h"

#define BSP_LOG_PATH "/data/bsplog/"

static int ref = 0;
zlog_category_t *g_zc = NULL;

int log_init(const char* conf_path)
{
	ref++;
	if (g_zc) return 0;

	if (access(BSP_LOG_PATH, 0)) {
		if(mkdir(BSP_LOG_PATH, 0777)){
			_log_error("mkdir %s error\n", BSP_LOG_PATH);
			return -1;
		}
	}

	int rc = zlog_init(conf_path);
	if (rc) {
		fprintf(stderr, "zlog init failed\n");
		// return -1;
	}

	g_zc = zlog_get_category("TIME");
	if (!g_zc) {
		fprintf(stderr, "zlog get category failed\n");
		zlog_fini();
		return -2;
	}

	_log_info("log init success.\n");

	return 0;
}

void log_fini(void)
{
	ref--;
	if (ref == 0)
		zlog_fini();
}


