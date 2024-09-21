/***********************************************************************
 * @file redisdb_err.h
 * @brief redisdb error object
 *
 * @author master@mapaware.top
 * @date 2024-09-22
 * @version 0.0.1
 * @copyright mapaware.top
 * @note
 **********************************************************************/
#ifndef REDISDB_ERR_H_INCL
#define REDISDB_ERR_H_INCL

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/param.h>

#include <common/cstrbuf.h>

#include <hiredis/hiredis.h>
#include <hiredis/async.h>

#include "redisdb_api.h"

#ifndef MAXPASSWORDLEN
#  define  MAXPASSWORDLEN  32
#endif


typedef struct _RedisdbErr
{
    int errcode;
    char errinfo[255];
} RedisdbErr;

// 全局变量定义在 .h 文件, 需要 extern 修饰
extern __thread RedisdbErr thread_local_err;

#if defined(__cplusplus)
}
#endif
#endif /* REDISDB_ERR_H_INCL */