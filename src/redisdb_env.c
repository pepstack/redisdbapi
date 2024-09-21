/***********************************************************************
 * @file redisdb_env.c
 * @brief redisdb environment
 *
 * @author master@mapaware.top
 * @date 2024-09-21
 * @version 0.0.1
 * @copyright mapaware.top
 * @note
 **********************************************************************/
#include "redisdb_env.h"

// 全局变量定义在 .h 文件
__thread RedisdbErr thread_local_err = {0};


RDBEnv RDBEnvCreate(const char *cluster_nodes, const char *auth_pass)
{
    RedisdbEnv *env = NULL;

    int numnodes = 0;
    char pass[MAXPASSWORDLEN] = {0};
    char host[80];
    int slen, port;

    slen = cstr_length(auth_pass, -1);
    if (! cstr_startwith(auth_pass, slen, "file://", 7)) {
        if (slen > sizeof(pass)) {
            perror("auth_pass is too long");
            return NULL;
        }
        memcpy(pass, auth_pass, slen);
    }

    slen = cstr_length(cluster_nodes, -1);
    if (cstr_startwith(cluster_nodes, slen, "file://", 7)) {
        // is cluster node file
        FILE *fp = fopen(&cluster_nodes[7], "r");
        char *pport;
        while(fscanf(fp, "%80s", host) != EOF) {
            numnodes++;
        }
        if (fseek(fp, 0, SEEK_SET)) {
            perror("Error rewinding file");
            fclose(fp);
            exit(-1);
        }

        env = (RedisdbEnv *) RDBMemAlloc(sizeof(RedisdbEnv) + sizeof(RedisdbCtxNode) * numnodes);
        assert(env->num_nodes == 0);
        while (env->num_nodes < numnodes) {
            if (fscanf(fp, "%80s", host) != EOF) {
                pport = strchr(host, ':');
                if (pport && pport != host) {
                    if (pport - host < MAXHOSTNAMELEN) {
                        host[ 80 - 1 ] = 0;
                        *pport++ = 0;
                        env->nodes[env->num_nodes].port = atoi(pport);
                        memcpy(env->nodes[env->num_nodes].host, host, pport - host);
                    }
                    memcpy(env->nodes[env->num_nodes].auth, auth_pass, MAXPASSWORDLEN);
                    env->num_nodes++;
                }
            }
        }

        if (env->num_nodes != numnodes) {
            perror("Unexcept error while reading file");
            fclose(fp);
            RDBMemFree(env);
            exit(-1);
        }

    } else {
        // host:port host:port host:port ...


    }

    thread_local_err.errcode = -1;
    thread_local_err.errinfo[0] = '\0';

    return (RDBEnv) env;
}


void RDBEnvFree(RDBEnv env)
{
    RedisdbEnv * envp = (RedisdbEnv *) env;
    if (envp) {
        for (int i = 0; i < envp->num_nodes; i++) {
            RDBCtxNodeFree(&envp->nodes[i]);
        }
        RDBMemFree(env);
    }
}
