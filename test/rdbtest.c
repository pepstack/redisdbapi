/***********************************************************************
 * @file rdbtest.c
 * @brief test redisdb api for debug libredisdb
 *
 * @author master@mapaware.top
 * @since 2024-09-22
 * @date 2025-08-12
 * @version 0.0.1
 * @copyright mapaware.top
 * @note
 *  hiredis_ssl:
 *     https://blog.51cto.com/u_16213405/7654873
 **********************************************************************/
#include "redisdb_api.h"

#include <common/unitypes.h>

// 手工设置下面的值:
#define CLUSTER_ALL_NODES  "file:///opt/redis_cluster/radius/CLUSTER_ALL_NODES"
#define CLUSTER_AUTH_PASS  "file:///opt/redis_cluster/radius/CLUSTER_AUTH_PASS"

int main(int argc, char *argv[])
{
    RDBEnv env;
    RDBCtxNode node;

    printf("CLUSTER_ALL_NODES: %s\n", CLUSTER_ALL_NODES);
    printf("CLUSTER_AUTH_PASS: %s\n", CLUSTER_AUTH_PASS);

    env = RDBEnvCreate(CLUSTER_ALL_NODES, CLUSTER_AUTH_PASS);
    if (! env) {
        exit(-1);
    }

    int nodes = RDBEnvGetNodes(env);
    printf("all nodes=%d\n", nodes);

    node = RDBEnvGetNodeAt(env, 0);

    node = RDBEnvConnectNode(env, node, 0, 0);

    RDBEnvFree(env);
    return 0;
}