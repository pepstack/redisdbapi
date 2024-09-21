/***********************************************************************
 * @file redisdb_ctx.h
 * @brief redisdb context
 *
 * @author master@mapaware.top
 * @date 2024-09-21
 * @version 0.0.1
 * @copyright mapaware.top
 * @note
 **********************************************************************/
#ifndef REDISDB_CTX_H_INCL
#define REDISDB_CTX_H_INCL

#if defined(__cplusplus)
extern "C" {
#endif

#include "redisdb_err.h"

#ifndef MAXPASSWORDLEN
#  define  MAXPASSWORDLEN  32
#endif


typedef struct _RedisdbCtxNode
{
    redisContext *ctx;

    int conn_timeo_ms;
    int io_timeo_ms;

    // 服务端口号
    int port;

    // 主机名. MAXHOSTNAMELEN 是此数组的大小，如果有空间，主机名以空字符结束
    char host[MAXHOSTNAMELEN];

    // 认证密码. MAXPASSWORDLEN 是此数组的大小，如果有空间，密码以空字符结束
    char auth[MAXPASSWORDLEN];
} RedisdbCtxNode;


void RDBCtxNodeFree(RedisdbCtxNode *ctxnode);



#if defined(__cplusplus)
}
#endif
#endif /* REDISDB_CTX_H_INCL */