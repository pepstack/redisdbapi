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


int main(int argc, char *argv[])
{
    char path[256];

    snprintf(path, 255, "file://%s/CLUSTER_ALL_NODES", getenv("REDIS_CLUSTER_HOME"));

    printf("CLUSTER_ALL_NODES=%s\n", path);

    RDBEnv env = RDBEnvCreate(path, "red123");
    if (! env) {
        exit(-1);
    }

    //

    RDBEnvFree(env);
    return 0;
}