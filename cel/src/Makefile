# CEL(C Extension Library) Makefile
# Copyright ((C)2008 - 2020 Hu Jinya <hu_jinya@163.com>
#
# This program is free software; you can redistribute it and/or 
# modify it under the terms of the GNU General Public License 
# as published by the Free Software Foundation; either version 2 
# of the License, or (at your option) any later version. 
# 
# This program is distributed in the hope that it will be useful, 
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
# GNU General Public License for more details.

HOST_ARCH := $(shell uname -m)
ARCH ?= $(HOST_ARCH)

ifeq ($(ARCH), aarch64)
	CROSS =aarch64-linux-gnu-
	LDFLAGS = -L../../mysql/lib_aarch64 -L../../openssl/lib_aarch64
	A_TARGET = ../lib_aarch64/libcel.a
	SO_TARGET = ../lib_aarch64/libcel.so
	BIN_TARGET = 
	OBJ_DIR = ../obj_aarch64/src
else ifeq ($(ARCH), x86_64)
	CROSS =
	LDFLAGS = -L../../mysql/lib -L../../openssl/lib
	A_TARGET = ../lib/libcel.a
	SO_TARGET = ../lib/libcel.so
	BIN_TARGET = 
	OBJ_DIR = ../obj/src
endif
CXX = $(CROSS)gcc
STRIP = $(CROSS)strip
AR = $(CROSS)ar

CXXFLAGS = -Wall -Wno-deprecated -fPIC  -g  -I ../../cel/include -I ../../mysql/include -I ../../openssl/include
LIBS = -lpthread -lcrypt -l:libmysqlclient.so.21.2.39 -l:libssl.so.3.0.13 -l:libcrypto.so.3.0.13

MAJOR = 1
MINOR = 6
REVISION = 0
BUILD = 80
EXTRA = ""
VERSION_FILE= "../include/cel/_version.h"

SUB_DIRS = 
SRC_DIRS = _unix sys crypto net sql
SRC_EXTS = .c .cc .cpp .cxx 

include ../tmpl.mk
