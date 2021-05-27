/*************************************************************************
	> File Name: log.h
	> Author: XXDK
	> Email: v.manstein@qq.com
	> Created Time: Fri 01 Nov 2019 10:28:02 AM CST
 ************************************************************************/

#ifndef _LOG_H
#define _LOG_H

#include "zlog.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USE_ZLOG

#ifdef USE_ZLOG
#define _log_info(...)    do {if(g_zc) zlog_info(g_zc,__VA_ARGS__);}while(0)
#define _log_error(...)   do {if(g_zc) zlog_error(g_zc,__VA_ARGS__);}while(0)
#define _log_warn(...)    do {if(g_zc) zlog_warn(g_zc,__VA_ARGS__);}while(0)
#define _log_fatal(...)   do {if(g_zc) zlog_fatal(g_zc,__VA_ARGS__);}while(0)
#define _log_notice(...)  do {if(g_zc) zlog_notice(g_zc,__VA_ARGS__);}while(0)
#define _log_debug(...)   do {if(g_zc) zlog_debug(g_zc,__VA_ARGS__);}while(0)
#else
#define _log_info(...)    fprintf(stdout, __VA_ARGS__)
#define _log_error(...)   fprintf(stdout, __VA_ARGS__)
#define _log_warn(...)    fprintf(stdout, __VA_ARGS__)
#define _log_fatal(...)   fprintf(stdout, __VA_ARGS__)
#define _log_notice(...)  fprintf(stdout, __VA_ARGS__)
#define _log_debug(...)   fprintf(stdout, __VA_ARGS__)
#endif

extern zlog_category_t *g_zc;
extern int log_init(const char* conf_path);
extern void log_fini(void);

#ifdef __cplusplus
}
#endif

#endif // _LOG_H
