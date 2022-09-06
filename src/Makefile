# Redis Makefile
# Copyright (C) 2009 Salvatore Sanfilippo <antirez at gmail dot com>
# This file is released under the BSD license, see the COPYING file
#
# The Makefile composes the final FINAL_CFLAGS and FINAL_LDFLAGS using
# what is needed for Redis plus the standard CFLAGS and LDFLAGS passed.
# However when building the dependencies (Jemalloc, Lua, Hiredis, ...)
# CFLAGS and LDFLAGS are propagated to the dependencies, so to pass
# flags only to be used when compiling / linking Redis itself REDIS_CFLAGS
# and REDIS_LDFLAGS are used instead (this is the case of 'make gcov').
#
# Dependencies are stored in the Makefile.dep file. To rebuild this file
# Just use 'make dep', but this is only needed by developers.

REDIS_SERVER_NAME=redis-server$(PROG_SUFFIX)
REDIS_SERVER_OBJ=server.o

# 所有要编译的文件
all: $(REDIS_SERVER_NAME)

# 编译redis-server,对应REDIS_SERVER_OBJ的o文件
$(REDIS_SERVER_NAME): $(REDIS_SERVER_OBJ)

clean:
	rm -rf $(REDIS_SERVER_NAME) *.o
