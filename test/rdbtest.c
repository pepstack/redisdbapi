/***********************************************************************
 * @file rdbtest.c
 * @brief test redisdb api for debug libredisdb
 *
 * @author master@mapaware.top
 * @date 2024-09-22
 * @version 0.0.1
 * @copyright mapaware.top
 * @note
 *  hiredis_ssl:
 *     https://blog.51cto.com/u_16213405/7654873
 **********************************************************************/
#include "redisdb_api.h"

#include <common/unitypes.h>


int main(int argc, char *argv[])
{
    char nodesfile[256];
    char authfile[256];

    RDBEnv env;
    RDBCtxNode node;

    snprintf_chk_abort(nodesfile, sizeof(nodesfile), "file://%s/CLUSTER_ALL_NODES", getenv("REDIS_CLUSTER_HOME"));
    snprintf_chk_abort(authfile, sizeof(authfile), "file://%s/REDIS-AUTH-PASSWORD", getenv("REDIS_CLUSTER_HOME"));

    printf("all nodes file: %s\n", nodesfile);
    printf("auth pass file: %s\n", authfile);

    env = RDBEnvCreate(nodesfile, authfile);
    if (! env) {
        exit(-1);
    }
    RDBEnvFree(env);


    env = RDBEnvCreate("hacl-node1:6377,hacl-node1:6378,hacl-node1:6379", authfile);
    if (! env) {
        exit(-1);
    }

    int nodes = RDBEnvGetNodes(env);
    printf("all nodes=%d\n", nodes);

    RDBEnvConnectNode(env, NULL, 0, 0);

    RDBEnvFree(env);
    return 0;
}