/***********************************************************************
 * @file redisdb_env.h
 * @brief redisdb environment struct
 *
 * @author master@mapaware.top
 * @date 2024-09-21
 * @version 0.0.1
 * @copyright mapaware.top
 * @note
 **********************************************************************/
#ifndef REDISDB_ENV_H_INCL
#define REDISDB_ENV_H_INCL

#if defined(__cplusplus)
extern "C" {
#endif

#include "redisdb_ctx.h"


typedef struct RedisdbEnv_t
{
    int num_nodes;
    RedisdbCtxNode nodes[0];
} RedisdbEnv;


#if defined(__cplusplus)
}
#endif
#endif /* REDISDB_ENV_H_INCL */