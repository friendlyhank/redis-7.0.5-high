
#include "config.h"

#include <string.h>

/*内存的分配获取panic Allocate memory or panic */
void *zmalloc(size_t size) {

}

// 字符串复制
char *zstrdup(const char *s) {
    size_t l = strlen(s)+1;// 计算大小
    char *p = zmalloc(l); // 分配内存

    memcpy(p,s,l); // 拷贝值
    return p;
}