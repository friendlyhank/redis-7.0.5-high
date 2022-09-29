
#define redisAtomic _Atomic

#include <stdatomic.h>
// 原子操作增加
#define atomicIncr(var,count) atomic_fetch_add_explicit(&var,(count),memory_order_relaxed)