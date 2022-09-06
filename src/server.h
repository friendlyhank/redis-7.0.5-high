
struct redisServer{
    /* Configuration */
    int verbosity;                  /*日志等级 Loglevel in redis.conf */
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