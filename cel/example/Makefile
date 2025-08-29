# CEL(C Extension Library Example) Makefile
# Copyright (c) 2008 - 2018 Hu Jinya <hu_jinya@163.com>

HOST_ARCH := $(shell uname -m)
ARCH ?= $(HOST_ARCH)

ifeq ($(ARCH), aarch64)
	CROSS =aarch64-linux-gnu-
	LDFLAGS = -L../../mysql/lib_aarch64 -L../../openssl/lib_aarch64 -L ../../cel/lib_aarch64/
	A_TARGET =
	SO_TARGET =
	BIN_TARGET = ../bin_aarch64/cel-example
	OBJ_DIR = ../obj_aarch64/example
else ifeq ($(ARCH), x86_64)
	CROSS =
	LDFLAGS = -L../../mysql/lib -L../../openssl/lib -L ../../cel/lib/
	A_TARGET =
	SO_TARGET =
	BIN_TARGET = ../bin/cel-example
	OBJ_DIR = ../obj/example
endif

CXX = $(CROSS)gcc
STRIP = $(CROSS)strip
AR = $(CROSS)ar

CXXFLAGS = -Wall -Wno-deprecated -fPIC  -g  -I ../../cel/include -I ../../mysql/include -I ../../openssl/include
LIBS = -lpthread -lcrypt -l:libssl.so.3 -l:libcrypto.so.3 -l:libcel.a


MAJOR = 0
MINOR = 0
REVISION = 0
BUILD = 0
EXTRA = ""
VERSION_FILE= 

SUB_DIRS = 
SRC_DIRS = 
SRC_EXTS = .c .cc .cpp .cxx 


include ../tmpl.mk
