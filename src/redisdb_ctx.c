/***********************************************************************
 * @file redisdb_ctx.c
 * @brief redisdb context
 *
 * @author master@mapaware.top
 * @date 2024-09-21
 * @version 0.0.1
 * @copyright mapaware.top
 * @note
 **********************************************************************/
#include "redisdb_ctx.h"


void RDBCtxNodeFree(RedisdbCtxNode *ctxnode)
{
    if (ctxnode) {
        redisContext *ctx = ctxnode->ctx;
        ctxnode->ctx = NULL;
        if (ctx) {
            redisFree(ctx);
        }
    }
}