/*
 * Copyright (c) 2009-2012, Salvatore Sanfilippo <antirez at gmail dot com>
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <syslog.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <signal.h>

#include "sds.h"     /*动态安全字符串 Dynamic safe strings */
#include "dict.h"    /*hash表 Hash tables */
#include "zmalloc.h" /*内存分配 total memory usage aware version of malloc/free */

/*静态服务配置 Static server configuration */
#define LOG_MAX_LEN    1024 /*消息的最大长度 Default maximum length of syslog messages.*/

/*日志等级 Log levels */
#define LL_DEBUG 0
#define LL_VERBOSE 1
#define LL_NOTICE 2
#define LL_WARNING 3
#define LL_RAW (1<<10) /* Modifier to log without timestamp */

struct redisServer {
    /*常规参数 General */
    pid_t pid;                  /* Main process pid. */
    int sentinel_mode;          /*是否哨兵模式 True if this instance is a Sentinel. */

    /* Configuration */
    int verbosity;                  /*配置中的日志等级 Loglevel in redis.conf */

    /* Logging */
    char *logfile;                  /*日志文件路径 Path of log file */
    int syslog_enabled;             /*是否系统日志输出 Is syslog enabled? */

    /* Replication (slave) */
    char *masterhost;               /*主节点的host Hostname of master */
    int masterport;                 /*主节点的port Port of master */

    /*时间缓存 time cache */
    time_t timezone;            /*时区 Cached timezone. As set by tzset(). */
    int daylight_active;        /* Currently in daylight saving time. */
};

/*-----------------------------------------------------------------------------
 * Extern declarations
 *----------------------------------------------------------------------------*/

extern struct redisServer server;
extern dictType sdsHashDictType;

/* Core functions */
void _serverLog(int level, const char *fmt, ...)
__attribute__((format(printf, 2, 3)));
void serverLogRaw(int level, const char *msg);

/*配置信息 Configuration */
/*配置信息标志 Configuration Flags */
#define IMMUTABLE_CONFIG (1ULL<<0) /*是否在启动的时候设置 Can this value only be set at startup? */

/*不同类型的配置枚举 Type of configuration. */
typedef enum {
    BOOL_CONFIG,
    STRING_CONFIG,
    SDS_CONFIG,
} configType;

/*配置信息相关 Type of configuration. */
struct rewriteConfigState; /*重写配置的状态 Forward declaration to export API. */
void initConfigValues();

/* Use macro for checking log level to avoid evaluating arguments in cases log
 * should be ignored due to low level. */
#define serverLog(level, ...) do {\
        if (((level)&0xff) < server.verbosity) break;\
        _serverLog(level, __VA_ARGS__);\
    } while(0)
