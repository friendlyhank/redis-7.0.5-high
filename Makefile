# Top level makefile, the real shit is at src/Makefile

# 真正编译是在src/Makefile文件
default: all

.DEFAULT:
	cd src && $(MAKE) $@

install:
	cd src && $(MAKE) $@

.PHONY: install