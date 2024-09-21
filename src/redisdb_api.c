/***********************************************************************
 * @file redisdb_api.c
 * @brief redis db client api implement
 *
 * @author master@mapaware.top
 * @date 2024-09-21
 * @version 0.0.1
 * @copyright mapaware.top
 * @note
 **********************************************************************/
#include "redisdb_api.h"
#include "redisdb_env.h"

const char * RDBLibVersion(void)
{
    return "libredisdb-0.0.1";
}


void * RDBMemAlloc(size_t sizeb)
{
    return mem_alloc_zero(1, sizeb);
}


void * RDBMemRealloc(void *old_addr, size_t old_sizeb, size_t new_sizeb)
{
    void *new_addr = mem_realloc(old_addr, new_sizeb);

    if (old_sizeb != (size_t)(-1) && new_sizeb > old_sizeb) {
        bzero((char*)new_addr + old_sizeb, new_sizeb - old_sizeb);
    }

    return new_addr;
}


void RDBMemFree(void *addr)
{
    mem_free(addr);
}