
/* Log levels 日志等级 */
#define LL_DEBUG 0
#define LL_NOTICE 2
#define LL_WARNING 3

int main(int argc, char **argv) {
//    serverLog(LL_WARNING, "oO0OoO0OoO0Oo Redis is starting oO0OoO0OoO0Oo");
}

/* Use macro for checking log level to avoid evaluating arguments in cases log
 * should be ignored due to low level. */
// 服务端日志打印
//#define serverLog(level, ...) do {\
//        if (((level)&0xff) < server.verbosity) break;\
//        _serverLog(level, __VA_ARGS__);\
//    } while(0)

