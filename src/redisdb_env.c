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


static int RedisAuthNode(redisContext *ctx, const char *authpass)
{
    redisReply *reply = (redisReply *) redisCommand(ctx, "AUTH %*s", cstr_length(authpass, MAXPASSWORDLEN), authpass);
    if (! reply) {
        return REDISDB_API_ERROR;
    }

    if (reply->type == REDIS_REPLY_ERROR) {
        freeReplyObject(reply);
        return REDISDB_API_ERROR;
    }

    if (reply->len == 2 && reply->str[0] == 'O' && reply->str[1] == 'K') {
        freeReplyObject(reply);
        return REDISDB_API_SUCCESS;
    }

    freeReplyObject(reply);
    return REDISDB_API_ERROR;
}


static int RedisConnectNode(RDBCtxNode node, int conn_ms, int io_ms)
{
    redisReply *reply;
    redisContext *ctx = node->ctx;
    node->ctx = NULL;

    if (ctx) {
        if (node->auth[0]) {
            if (RedisAuthNode(ctx, node->auth) == REDISDB_API_ERROR) {
                redisFree(ctx);
                return REDISDB_API_ERROR;
            }
        }

        if (node->io_timeo_ms != io_ms && io_ms) {
            // io timeout changed
            struct timeval tvio = { io_ms / 1000, (io_ms % 1000) * 1000 };
            if (redisSetTimeout(ctx, tvio) == REDIS_ERR) {
                // set timeout failed
                redisFree(ctx);
                return REDISDB_API_NEXT;
            }
        }

        reply = redisCommand(ctx, "PING");
        if (! reply) {
            redisFree(ctx);
            return REDISDB_API_NEXT;
        }

        if (reply->type == REDIS_REPLY_ERROR) {
            freeReplyObject(reply);
            redisFree(ctx);
            return REDISDB_API_ERROR;
        }

        if (cstr_compare_len(reply->str, reply->len, "PONG", 4, 0)) {
            freeReplyObject(reply);
            redisFree(ctx);
            return REDIS_REPLY_ERROR;
        }

        // all is ok
        freeReplyObject(reply);

        node->io_timeo_ms = io_ms;
        node->conn_timeo_ms = conn_ms;
        node->ctx = ctx;

        return REDISDB_API_SUCCESS;
    }

    assert(ctx == NULL);
    if (conn_ms) {
        struct timeval tvconn = { conn_ms / 1000, (conn_ms % 1000) * 1000 };
        ctx = redisConnectWithTimeout(node->host, node->port, tvconn);
    } else {
        ctx = redisConnect(node->host, node->port);
    }
    if (! ctx) {
        // Error connect
        return REDISDB_API_ERROR;
    }

    node->ctx = ctx;
    return RedisConnectNode(node, conn_ms, io_ms);
}


/**
 * @brief 
 * 
 * @param cluster_nodes 
 * @param auth_pass 
 * @return RDBEnv 
 */
RDBEnv RDBEnvCreate(const char *cluster_nodes, const char *auth_pass)
{
    RedisdbEnv *env = NULL;

    int numnodes = 0;
    char pass[MAXPASSWORDLEN + 1] = {0};
    char host[80];
    int i, slen, port;
    char *pport;

    // 获取密码, 密码可能来自下面之一:
    //  1) 文件(file:///path/to/authpassfile)
    //  2) 环境变量(env:varname)
    //  3) 本身就是密码(password)
    slen = cstr_length(auth_pass, -1);
    if (cstr_startwith(auth_pass, slen, "file://", 7)) {
        // is auth_pass file
        FILE *fp = fopen(&auth_pass[7], "r");
        if (! fp) {
            perror("Error open file");
            exit(-1);
        }
        fscanf(fp, "%32s", pass);
        fclose(fp);
    } else if (cstr_startwith(auth_pass, slen, "env:", 4)) {
        snprintf(pass, MAXPASSWORDLEN, "%s", getenv(&auth_pass[4]));
    } else if (slen < sizeof(pass)) {
        snprintf(pass, MAXPASSWORDLEN, "%s", auth_pass);
    } else {
        perror("auth_pass is too long");
        return NULL;
    }

    // 获取节点配置, 可能来自下面之一:
    //  1) 文件(file:///path/to/authpassfile)
    //  2) 环境变量(env:varname)
    //  3) 本身就是(host:port,host:port,...)
    slen = cstr_length(cluster_nodes, -1);
    if (cstr_startwith(cluster_nodes, slen, "file://", 7)) {
        // is cluster node file
        FILE *fp = fopen(&cluster_nodes[7], "r");
        if (! fp) {
            perror("Error open file");
            exit(-1);
        }

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

        fclose(fp);

        if (env->num_nodes != numnodes) {
            perror("Unexcept error while reading file");
            RDBMemFree(env);
            exit(-1);
        }
    } else {
        char hostports[1024] = {0};
        char *pairs[30];
        int  plens[30];

        if (cstr_startwith(cluster_nodes, slen, "env:", 4)) {
            // env:环境变量
            slen = snprintf(hostports, sizeof(hostports), "%s", getenv(&cluster_nodes[4]));
        } else {
            // host:node,host:node,...
            slen = snprintf(hostports, sizeof(hostports), "%s", cluster_nodes);
        }
        if (slen < 4 || slen >= sizeof(hostports)) {
            perror("Error setting for cluster nodes");
            exit(-1);
        }

        // 解析字符串 hostports, 得到 host 和 port
        numnodes = cstr_slpit_chr_nodup(hostports, slen, ',', pairs, plens, sizeof(pairs)/sizeof(pairs[0]));
        env = (RedisdbEnv *) RDBMemAlloc(sizeof(RedisdbEnv) + sizeof(RedisdbCtxNode) * numnodes);
        assert(env->num_nodes == 0);
        i = 0;
        while (i < numnodes) {
            char *hp = pairs[env->num_nodes];
            int hostlen, portlen;

            pport = strchr(hp, ':');

            if (! pport) {
                // 1234
                pport = hp;
                portlen = plens[i];
                hostlen = strlen("127.0.0.1");
                cstr_copybuf_len(env->nodes[i].host, "127.0.0.1", hostlen);
            } else if (pport == hp) {
                // :1234
                pport++;
                portlen = plens[i] - 1;
                hostlen = strlen("127.0.0.1");
                cstr_copybuf_len(env->nodes[i].host, "127.0.0.1", hostlen);
            } else {
                // 192.168.56.111:1234
                pport++;
                hostlen = pport - hp - 1;
                portlen = plens[i] + (hp - pport);
                cstr_copybuf_len(env->nodes[i].host, hp, hostlen);
            }

            cstr_copybuf_len(host, pport, portlen);
            env->nodes[i].port = atoi(host);

            i++;
        }

        env->num_nodes = i;
        if (env->num_nodes != numnodes) {
            perror("Unexcept error while parse config");
            RDBMemFree(env);
            exit(-1);
        }
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


int RDBEnvGetNodes(RDBEnv env)
{
    return ((RedisdbEnv *) env)->num_nodes;
}


RDBCtxNode RDBEnvGetNodeAt(RDBEnv env, int index)
{
    RDBCtxNode node = NULL;
    RedisdbEnv * pEnv = (RedisdbEnv *) env;
    if (index >= 0 && index < pEnv->num_nodes) {
        node = (RDBCtxNode) (&pEnv->nodes[index]);
    }
    return node;
}


RDBCtxNode RDBEnvConnectNode(RDBEnv env, RDBCtxNode connnode, int conn_timeout_ms, int io_timeout_ms)
{
    RedisdbEnv * pEnv = (RedisdbEnv *) env;

    if (! connnode) {
        // connect all nodes and returns the first connected node
        for (int i = 0; i < pEnv->num_nodes; i++) {
            RedisConnectNode(&pEnv->nodes[i], conn_timeout_ms, io_timeout_ms);
        }
    } else {
        // connect only the node
        RedisConnectNode(connnode, conn_timeout_ms, io_timeout_ms);
    }

    return NULL;
}


int RDBEnvDisconnectNode(RDBEnv env, RDBCtxNode node)
{
    RedisdbEnv * pEnv = (RedisdbEnv *) env;

    if (node) {
        RDBCtxNodeFree(node);
        return 0;
    }

    // disconnect all nodes
    for (int i = 0; i < pEnv->num_nodes; i++) {
        RDBEnvDisconnectNode(env, &pEnv->nodes[i]);
    }

    return 0;
}