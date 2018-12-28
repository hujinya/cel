# CEL(C Extension Library) Makefile
# Copyright (c) 2008 - 2018 Hu Jinya <hu_jinya@163.com>
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

CROSS =
CXX = $(CROSS)gcc
STRIP = $(CROSS)strip

CXXFLAGS = -Wall -Wno-deprecated -fPIC  -g  -I ../../cel/include
LDFLAGS = -L/usr/lib64/mysql
LIBS = -lpthread -lmysqlclient -lcrypt -lssl -lcrypto

A_TARGET = ../lib/libcel.a
SO_TARGET = ../lib/libcel.so
BIN_TARGET = 

MAJOR = 1
MINOR = 1
REVISION = 0
BUILD = 31
EXTRA = ""
VERSION_FILE= "../include/cel/_version.h"

SUB_DIRS = 
SRC_DIRS = _unix sys crypto net sql
SRC_EXTS = .c .cc .cpp .cxx 
OBJ_DIR = ../obj/src

include ../tmpl.mk
