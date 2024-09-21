/***********************************************************************
 * @file redisdb_api.h
 * @brief redis db client api
 *
 *   api wrapper of hiredis to make redis apllication developing easier.
 * @author master@mapaware.top
 * @date 2024-09-21
 * @version 0.0.1
 * @copyright mapaware.top
 * @note
 **********************************************************************/
#ifndef REDISDB_API_H_PUBLIC
#define REDISDB_API_H_PUBLIC

#if defined(__cplusplus)
extern "C" {
#endif


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#if defined(REDISDB_DLL)
/* win32 dynamic dll */
# ifdef REDISDB_EXPORTS
#   define REDISDB_API __declspec(dllexport)
# else
#   define REDISDB_API __declspec(dllimport)
# endif
#else
/* static lib or linux so */
# define REDISDB_API  extern
#endif

#define REDISDB_API_SUCCESS   (0)
#define REDISDB_API_ERROR    (-1)

#define REDISDB_API_NEXT      (1)

typedef struct _RedisdbEnv        * RDBEnv;
typedef struct _RedisdbCtxNode    * RDBCtxNode;


REDISDB_API const char * RDBLibVersion(void);

REDISDB_API void * RDBMemAlloc(size_t sizeb);

REDISDB_API void * RDBMemRealloc(void *old_addr, size_t old_sizeb, size_t new_sizeb);

REDISDB_API void RDBMemFree(void *addr);


/**
 * @brief Create RDBEnv Object
 * 
 * @param cluster_nodes 
 * @return A handle to RDBEnv Object
 */
REDISDB_API RDBEnv RDBEnvCreate(const char *cluster_nodes, const char *auth_pass);


/**
 * @brief 
 * 
 * @param env 
 * @return REDISDB_API 
 */
REDISDB_API void RDBEnvFree(RDBEnv env);

REDISDB_API int RDBEnvGetNodes(RDBEnv env);

REDISDB_API RDBCtxNode RDBEnvGetNodeAt(RDBEnv env, int index);

REDISDB_API RDBCtxNode RDBEnvConnectNode(RDBEnv env, RDBCtxNode node, int connect_timeout_ms, int io_timeout_ms);

REDISDB_API int RDBEnvDisconnectNode(RDBEnv env, RDBCtxNode node);

#if defined(__cplusplus)
}
#endif
#endif /* REDISDB_API_H_PUBLIC */