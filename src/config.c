/* Configuration file parsing and CONFIG GET/SET commands implementation.
 *
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
#include "server.h"

#include <fcntl.h>
#include <sys/stat.h>
#include <glob.h>
#include <string.h>

// 标准配置信息
typedef struct standardConfig standardConfig;

typedef int (*apply_fn)(const char **err);
// typeInterface - 根据不同数据类型初始化接口
typedef struct typeInterface {
    /*服务启动时，初始化默认值 Called on server start, to init the server with default value */
    void (*init)(standardConfig *config);
    /*
     * 服务启动或者在配置set的时候使用,1返回成功,2表示没有进行实际修改,0表示有错误并返回实际错误
     * Called on server startup and CONFIG SET, returns 1 on success,
     * 2 meaning no actual change done, 0 on error and can set a verbose err
     * string */
    int (*set)(standardConfig *config, sds *argv, int argc, const char **err);
    /* Optional: called after `set()` to apply the config change. Used only in
    * the context of CONFIG SET. Returns 1 on success, 0 on failure.
    * Optionally set err to a static error string. */
    apply_fn apply;
    /*配置需要重写的时候使用 Called on CONFIG REWRITE, required to rewrite the config state */
    void (*rewrite)(standardConfig *config, const char *name, struct rewriteConfigState *state);
} typeInterface;

// 标准配置信息结构体
struct standardConfig {
    const char *name; /* 配置名称 The user visible name of this config */
    const char *alias; /*配置的别名 An alias that can also be used for this config */
    unsigned int flags; /*特殊配置的标记 Flags for this specific config */
};

/*配置重写的状态 The config rewrite state. */
struct rewriteConfigState {

};

// embedCommonConfig - 通用配置信息
#define embedCommonConfig(config_name, config_alias, config_flags) \
    .name = (config_name), \
    .alias = (config_alias), \
    .flags = (config_flags),

#define embedConfigInterface(initfn, setfn, getfn, rewritefn, applyfn) .interface = { \
},

#define ALLOW_EMPTY_STRING 0

// createStringConfig - 创建string配置信息
#define createStringConfig(name, alias, flags, empty_to_null, config_addr, default, is_valid, apply) { \
    embedCommonConfig(name, alias, flags) \
    embedConfigInterface(stringConfigInit, stringConfigSet, stringConfigGet, stringConfigRewrite, apply) \
}

// 配置信息结构体
standardConfig static_configs[] = {
        createStringConfig("logfile", NULL, IMMUTABLE_CONFIG, ALLOW_EMPTY_STRING, server.logfile, "", NULL, NULL)
};

/* Initialize configs to their default values and create and populate the
 * runtime configuration dictionary.
 * 初始化默认配置信息并且创建配置字段
 * 配置信息使用hash表进行存储
 * */
void initConfigValues() {
    for (standardConfig *config = static_configs;config->name != NULL;config++){

    }
}