
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

/* Log levels */
#define LL_DEBUG 0
#define LL_VERBOSE 1
#define LL_NOTICE 2
#define LL_WARNING 3
#define LL_RAW (1<<10) /* Modifier to log without timestamp */

struct redisServer{
    /*配置相关 Configuration */
    int verbosity;                  /*日志等级 Loglevel in redis.conf */

    /*日志信息 Logging */
    char *logfile;                  /*日志文件路径 Path of log file */
};

/*-----------------------------------------------------------------------------
 * Extern declarations
 *----------------------------------------------------------------------------*/
extern struct redisServer server; // 生命server外部变量

/* Use macro for checking log level to avoid evaluating arguments in cases log
 * should be ignored due to low level. */
// 服务端日志打印
#define serverLog(level, ...) do {\
        if (((level)&0xff) < server.verbosity) break;\
        _serverLog(level, __VA_ARGS__);\
    } while(0)