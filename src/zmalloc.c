/* zmalloc - total amount of allocated memory aware version of malloc()
 *
 * Copyright (c) 2009-2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <assert.h>

#include <string.h>
#include "zmalloc.h"
#include "atomicvar.h"

#ifdef HAVE_MALLOC_SIZE
#define PREFIX_SIZE (0) // mac有计算已分配空间大小的函数，linux则没有，需要额外的空间存储大小
#define ASSERT_NO_SIZE_OVERFLOW(sz)
#else
#endif

/* When using the libc allocator, use a minimum allocation size to match the
 * jemalloc behavior that doesn't return NULL in this case.
 * 根据不同机器设置内存最小分配大小
 */
#define MALLOC_MIN_SIZE(x) ((x) > 0 ? (x) : sizeof(long))

#define update_zmalloc_stat_alloc(__n) atomicIncr(used_memory,(__n)) // 记录已分配内存大小
#define update_zmalloc_stat_free(__n) atomicDecr(used_memory,(__n))  // 记录已释放内存大小

static redisAtomic size_t used_memory = 0; // 全局变量，已使用内存大小

// 分配内存失败错误打印
static void zmalloc_default_oom(size_t size) {
    fprintf(stderr, "zmalloc: Out of memory trying to allocate %zu bytes\n",
            size);
    fflush(stderr);
    abort();// 终止程序
}

// 分配内存失败，打印异常
static void (*zmalloc_oom_handler)(size_t) = zmalloc_default_oom;

/* Try allocating memory, and return NULL if failed.
 * '*usable' is set to the usable size if non NULL.
 * 尝试分配内存，如果失败返回NULL
 * */
void *ztrymalloc_usable(size_t size, size_t *usable) {
    // 判断是否溢出
    ASSERT_NO_SIZE_OVERFLOW(size);
    void *ptr = malloc(MALLOC_MIN_SIZE(size)+PREFIX_SIZE);

    if(!ptr) return NULL;
#ifdef HAVE_MALLOC_SIZE
    size = zmalloc_size(ptr);
    update_zmalloc_stat_alloc(size);
     if (usable) *usable = size;
    return ptr;
#else

#endif
}

/*内存的分配获取panic Allocate memory or panic */
void *zmalloc(size_t size) {
    void *ptr = ztrymalloc_usable(size, NULL);
    if (!ptr) zmalloc_oom_handler(size);
    return ptr;
}

// 字符串复制
char *zstrdup(const char *s) {
    size_t l = strlen(s)+1;// 计算大小
    char *p = zmalloc(l); // 分配内存

    memcpy(p,s,l); // 拷贝值
    return p;
}