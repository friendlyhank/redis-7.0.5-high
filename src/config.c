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

// string类型的配置数据
typedef struct stringConfigData {
    char **config; /*具体配置字段的指针地址(用于赋值) Pointer to the server config this value is stored in. */
    const char *default_value; /*默认的值 Default value of the config on rewrite. */
    int convert_empty_to_null; /*是否允许为空值 Boolean indicating if empty SDS strings should
                                  be stored as a NULL value. */
} stringConfigData;

// 具体类型配置的数据
typedef union typeData {
    stringConfigData string;
} typeData;

// 标准配置信息
typedef struct standardConfig standardConfig;

typedef int (*apply_fn)(const char **err);
// typeInterface - 根据不同数据类型初始化接口
typedef struct typeInterface {
    /*服务启动初始化接口，初始化默认值 Called on server start, to init the server with default value */
    void (*init)(standardConfig *config);
    /*
     * 设置配置接口,1返回成功,2表示没有进行实际修改,0表示有错误并返回实际错误
     * Called on server startup and CONFIG SET, returns 1 on success,
     * 2 meaning no actual change done, 0 on error and can set a verbose err
     * string */
    int (*set)(standardConfig *config, sds *argv, int argc, const char **err);
    /* Optional: called after `set()` to apply the config change. Used only in
    * the context of CONFIG SET. Returns 1 on success, 0 on failure.
    * Optionally set err to a static error string. */
    apply_fn apply;
    /*获取配置信息接口 Called on CONFIG GET, returns sds to be used in reply */
    sds (*get)(standardConfig *config);
    /*重写配置接口 Called on CONFIG REWRITE, required to rewrite the config state */
    void (*rewrite)(standardConfig *config, const char *name, struct rewriteConfigState *state);
} typeInterface;

// 标准配置信息结构体
struct standardConfig {
    const char *name; /* 配置名称 The user visible name of this config */
    const char *alias; /*配置的别名 An alias that can also be used for this config */
    unsigned int flags; /*特殊配置的标记 Flags for this specific config */
    typeInterface interface; /*根据类型实现接口方法 The function pointers that define the type interface */
    typeData data; /*具体类型的配置数据 The type specific data exposed used by the interface */
    configType type; /*配置数据类型 The type of config this is. */
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
    .init = (initfn), \
    .set = (setfn), \
    .get = (getfn), \
    .rewrite = (rewritefn), \
    .apply = (applyfn) \
},

/* What follows is the generic config types that are supported. To add a new
 * config with one of these types, add it to the standardConfig table with
 * the creation macro for each type.
 *
 * Each type contains the following:
 * * A function defining how to load this type on startup.
 * * A function defining how to update this type on CONFIG SET.
 * * A function defining how to serialize this type on CONFIG SET.
 * * A function defining how to rewrite this type on CONFIG REWRITE.
 * * A Macro defining how to create this type.
 */

/*string类型配置初始化 String Configs */
static void stringConfigInit(standardConfig *config) {
    // 初始化默认值,如果允许为空并且没有没有值，则设置为NULL
    config->data.string.config = (config->data.string.convert_empty_to_null && !config->data.string.default_value) ? NULL : zstrdup(config->data.string.default_value);
}

/*string类型配置设置 */
static int stringConfigSet(standardConfig *config, sds *argv, int argc, const char **err) {

}

// stringConfigGet - 配置获取
static sds stringConfigGet(standardConfig *config) {

}

// string配置重写
static void stringConfigRewrite(standardConfig *config, const char *name, struct rewriteConfigState *state) {

}

#define ALLOW_EMPTY_STRING 0

// createStringConfig - 创建string配置信息
#define createStringConfig(name, alias, flags, empty_to_null, config_addr, default, is_valid, apply) { \
    embedCommonConfig(name, alias, flags) \
    embedConfigInterface(stringConfigInit, stringConfigSet, stringConfigGet, stringConfigRewrite, apply) \
    .type = STRING_CONFIG, \
    .data.string = {                                                                                   \
        .config = &(config_addr),                                                                          \
        .default_value = (default), \
        .convert_empty_to_null = (empty_to_null), \
     } \
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
        if(config->interface.init) config->interface.init(config);// 如果配置需要初始化，则调用初始化方法
    }
}