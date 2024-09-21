########################################################################
# Makefile
#   gcc build libredisdb.so on/for linux.
#
# @author: master@mapaware.top
# @date: 2024-09-21
# @version 0.0.1
# @copyright mapaware.top
# @note
#
# Show all predefinitions of gcc:
#
#   https://blog.csdn.net/10km/article/details/49023471
#
#   $ gcc -posix -E -dM - < /dev/null
#
########################################################################
# Linux, MSYS_NT, CYGWIN ...
shuname="$(shell uname)"

# TODO: make only for Linux now !
OSARCH=$(shell echo $(shuname)|awk -F '-' '{ print $$1 }')

# debug | release (default)
BUILD ?= RELEASE

# 32 | 64 (default)
BITS ?= 64

################################################################
# Architecture Configuration
ifeq ($(BUILD), DEBUG)
    # make BUILD=DEBUG
	CFLAGS += -D_DEBUG -g
else
    # default is release
	CFLAGS += -DNDEBUG -O3
	BUILD = RELEASE
endif

ifeq ($(BITS), 32)
	# make BITS=32
	CFLAGS += -m32
	LDFLAGS += -m32
else
    # default is 64 bits
	CFLAGS += -m64
	LDFLAGS += -m64
endif

################################################################
# Compiler Specific Configuration
CC = gcc

# for gcc-8+
# -Wno-unused-const-variable
CFLAGS += -std=gnu99 -D_GNU_SOURCE -fPIC -Wall -Wno-unused-function -Wno-unused-variable
#......

LDFLAGS += -lpthread -lm
#......

################################################################
# Project Specific Configuration
PREFIX := .

# Given dirs for all source (*.c) files
SRC_DIR = $(PREFIX)/src
TEST_DIR = $(PREFIX)/test
DEP_LIBS = $(PREFIX)/libs

COMMON_DIR = $(SRC_DIR)/common


#----------------------------------------------------------
# redisdb
REDISDB_VERSION = $(shell cat $(SRC_DIR)/VERSION)

REDISDB_STATICLIB = libredisdb.a.$(REDISDB_VERSION)
REDISDB_DYNAMICLIB = libredisdb.so.$(REDISDB_VERSION)

#----------------------------------------------------------
# add other projects here:
#


# Set all dirs for C source: './src/a ./src/b'
ALLCDIRS += $(SRC_DIR) \
	$(COMMON_DIR)
#

# Get pathfiles for C source files: './src/a/1.c ./src/b/2.c'
CSRCS := $(foreach cdir, $(ALLCDIRS), $(wildcard $(cdir)/*.c))

# Get names of object files: '1.o 2.o'
COBJS = $(patsubst %.c, %.o, $(notdir $(CSRCS)))


# Given dirs for all header (*.h) files
INCDIRS += -I$(PREFIX) \
	-I$(DEP_LIBS)/hiredis/include \
	-I$(SRC_DIR) \
	-I$(COMMON_DIR)
#
################################################################
# Build Target Configuration
.PHONY: all test clean help

all: $(REDISDB_STATICLIB).$(OSARCH) $(REDISDB_DYNAMICLIB).$(OSARCH)


# http://www.gnu.org/software/make/manual/make.html#Eval-Function
define COBJS_template =
$(basename $(notdir $(1))).o: $(1)
	$(CC) $(CFLAGS) -c $(1) $(INCDIRS) -o $(basename $(notdir $(1))).o
endef

$(foreach src,$(CSRCS),$(eval $(call COBJS_template,$(src))))
################################################################

#----------------------------------------------------------
$(REDISDB_STATICLIB).$(OSARCH): $(COBJS)
	rm -f $@
	rm -f $(REDISDB_STATICLIB)
	ar cr $@ $^
	ln -s $@ $(REDISDB_STATICLIB)

#----------------------------------------------------------
$(REDISDB_DYNAMICLIB).$(OSARCH): $(COBJS)
	$(CC) $(CFLAGS) -shared \
		-Wl,--soname=$(REDISDB_DYNAMICLIB) \
		-Wl,--rpath='/usr/local/lib:$(PREFIX):$(PREFIX)/libs:$(DEP_LIBS)/hiredis/lib' \
		-o $@ \
		$^ \
		-L$(DEP_LIBS)/hiredis/lib \
		-lhiredis \
		$(LDFLAGS)
	ln -s $@ $(REDISDB_DYNAMICLIB)


#----------------------------------------------------------
test: rdbtest.$(OSARCH)


# -lrt for Linux
rdbtest.$(OSARCH): $(TEST_DIR)/rdbtest.c
	@echo Building rdbtest.$(OSARCH)
	$(CC) $(CFLAGS) $< $(INCDIRS) \
	-Wl,--soname=$(REDISDB_DYNAMICLIB) \
	-Wl,--rpath='/usr/local/lib:$(PREFIX):$(PREFIX)/libs:$(DEP_LIBS)/hiredis/lib' \
	-o $@ \
	$(REDISDB_STATICLIB) \
	$(LDFLAGS) \
	-L$(DEP_LIBS)/hiredis/lib -lhiredis \
	-lrt
	ln -sf $@ rdbtest


#----------------------------------------------------------
clean:
	-rm -f *.stackdump
	-rm -f $(COBJS)
	-rm -f $(PREFIX)/$(REDISDB_STATICLIB)
	-rm -f $(PREFIX)/$(REDISDB_DYNAMICLIB)
	-rm -f $(PREFIX)/$(REDISDB_STATICLIB).$(OSARCH)
	-rm -f $(PREFIX)/$(REDISDB_DYNAMICLIB).$(OSARCH)
	-rm -f $(PREFIX)/rdbtest.$(OSARCH) $(PREFIX)/rdbtest

#----------------------------------------------------------
help:
	@echo
	@echo "Build all libs and apps on $(OSARCH) with the following commands:"
	@echo " make BUILD=DEBUG"
	@echo " make BITS=32"
	@echo