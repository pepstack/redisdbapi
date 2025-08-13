########################################################################
# Makefile
#   gcc build libredisdb.so only on/for linux.
#
# @author: master@mapaware.top
# @since: 2024-09-21
# @date: 2025-08-12
# @version 0.0.1
# @copyright mapaware.top
# @note
#
# 查看 gcc 所有预定义(https://blog.csdn.net/10km/article/details/49023471):
#   $ gcc -posix -E -dM - < /dev/null
#
# 查看GCC默认模式:
#   $ gcc -v 2>&1 | grep "Target"
########################################################################
# Linux, CYGWIN_NT, MSYS_NT, ...
shuname="$(shell uname -s)"

# Linux/MSYS_NT/SunOS/AIX
OSARCH=$(shell echo $(shuname)|awk -F '-' '{ print $$1 }')

# 检测系统架构: x86_64
ARCH := $(shell uname -m)

# 定义支持的 Linux 变体列表
SUPPORTED_LINUX_VARIANTS := Linux GNU/Linux

# 检查是否为支持的 Linux 系统
ifneq ($(filter $(OSARCH),$(SUPPORTED_LINUX_VARIANTS)),)
  # Linux 系统，继续执行
  $(info Detected Linux system: $(shuname))
else
  # 非 Linux 系统，报错退出
  $(error Unsupported operating system: $(shuname). This Makefile requires Linux.)
endif

###############################################################
# Compiler Specific Configuration
CC = gcc

INCLUDES = -I. -I/usr/include -I/usr/local/include

# 设置用于编译一个多线程安全的、启用严格警告、并遵循ISO C标准的程序(-pedantic)。
# 共享库必须包含: -fPIC
CFLAGS = -D_REENTRANT -std=gnu99 -D_GNU_SOURCE -Wall -fPIC

LDFLAGS = -L. -L/usr/lib64 -L/usr/lib/x86_64-linux-gnu -lpthread -lm

# default: RELEASE
# make BUILD=DEBUG
BUILD ?= RELEASE

# Architecture Configuration
ifeq ($(BUILD), DEBUG)
    # make BUILD=DEBUG
	CFLAGS += -DDEBUG -g
	BUILD = DEBUG
else
    # default is release
	CFLAGS += -DNDEBUG -O3
	BUILD = RELEASE
endif

# 仅在需要时添加 -m64
ifeq ($(findstring 64, $(ARCH)),)
    CFLAGS += -m64
    LDFLAGS += -m64
endif

################################################################
# Project Specific Configuration
PREFIX := .

# Given dirs for all source (*.c) files
SRC_DIR = $(PREFIX)/src
TEST_DIR = $(PREFIX)/test
DEPS_DIR = $(PREFIX)/deps

COMMON_DIR = $(SRC_DIR)/common

#----------------------------------------------------------
# redisdb
REDISDB_VERSION = $(shell cat $(SRC_DIR)/VERSION)

REDISDB_STATICLIB = libredisdb.a
REDISDB_DYNAMICLIB = libredisdb.so

#----------------------------------------------------------

# Set all dirs for C source: './src/a ./src/b'
ALLCDIRS += $(SRC_DIR) \
	$(COMMON_DIR)


# Get pathfiles for C source files: './src/a/1.c ./src/b/2.c'
CSRCS := $(foreach cdir, $(ALLCDIRS), $(wildcard $(cdir)/*.c))

# Get names of object files: '1.o 2.o'
COBJS = $(patsubst %.c, %.o, $(notdir $(CSRCS)))


# Given dirs for all header (*.h) files
INCDIRS += -I$(PREFIX) \
	-I$(DEPS_DIR)/include \
	-I$(SRC_DIR) \
	-I$(COMMON_DIR)

LDFLAGS += -L$(DEPS_DIR)/lib

################################################################
# Build Target Configuration
.PHONY: all test clean help

all: $(REDISDB_STATICLIB).$(REDISDB_VERSION) $(REDISDB_DYNAMICLIB).$(REDISDB_VERSION)


# http://www.gnu.org/software/make/manual/make.html#Eval-Function
define COBJS_template =
$(basename $(notdir $(1))).o: $(1)
	$(CC) $(CFLAGS) -c $(1) $(INCDIRS) -o $(basename $(notdir $(1))).o
endef

$(foreach src,$(CSRCS),$(eval $(call COBJS_template,$(src))))

################################################################

$(REDISDB_STATICLIB).$(REDISDB_VERSION): $(COBJS)
	rm -f $@
	rm -f $(REDISDB_STATICLIB)
	ar cr $@ $^
	ln -s $@ $(REDISDB_STATICLIB)


$(REDISDB_DYNAMICLIB).$(REDISDB_VERSION): $(COBJS)
	$(CC) $(CFLAGS) -shared \
		-Wl,--soname=$(REDISDB_DYNAMICLIB) \
		-Wl,--rpath='/usr/local/lib:$(PREFIX):$(DEPS_DIR)/lib' \
		-o $@ \
		$^ \
		-lhiredis \
		-lhiredis_ssl \
		$(LDFLAGS)
	ln -s $@ $(REDISDB_DYNAMICLIB)


#----------------------------------------------------------
test: rdbtest

# -lrt for Linux
rdbtest: $(TEST_DIR)/rdbtest.c
	@echo Building rdbtest
	$(CC) $(CFLAGS) $< $(INCDIRS) \
	-Wl,--soname=$(REDISDB_DYNAMICLIB) \
	-Wl,--rpath='/usr/local/lib:$(PREFIX):$(DEPS_DIR)/lib' \
	-o $@ \
	$(REDISDB_STATICLIB) \
	-lhiredis \
	-lhiredis_ssl \
	$(LDFLAGS) \
	-lrt

#----------------------------------------------------------
clean:
	-rm -f *.stackdump
	-rm -f $(COBJS)
	-rm -f $(PREFIX)/$(REDISDB_STATICLIB)
	-rm -f $(PREFIX)/$(REDISDB_DYNAMICLIB)
	-rm -f $(PREFIX)/$(REDISDB_STATICLIB).$(REDISDB_VERSION)
	-rm -f $(PREFIX)/$(REDISDB_DYNAMICLIB).$(REDISDB_VERSION)
	-rm -f $(PREFIX)/rdbtest

#----------------------------------------------------------
help:
	@echo
	@echo "Build for DEBUG with the following command:"
	@echo " make BUILD=DEBUG"
	@echo "Build for RELEASE with the following command:"
	@echo " make
	@echo
