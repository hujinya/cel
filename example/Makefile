# CEL(C Extension Library Example) Makefile
# Copyright (c) 2008 - 2018 Hu Jinya <hu_jinya@163.com>

CROSS =
CXX = $(CROSS)gcc
STRIP = $(CROSS)strip

CXXFLAGS = -Wall -Wno-deprecated -fPIC  -g  -I ../../cel/include
LDFLAGS = -L/usr/lib64/mysql
LIBS = -lpthread -lmysqlclient -lcrypt -lssl -lcrypto ../../cel/lib/libcel.a

A_TARGET =
SO_TARGET =
BIN_TARGET = ../bin/cel-example

MAJOR = 0
MINOR = 0
REVISION = 0
BUILD = 0
EXTRA = ""
VERSION_FILE= 

SUB_DIRS = 
SRC_DIRS = 
SRC_EXTS = .c .cc .cpp .cxx 
OBJ_DIR = ../obj/example

include ../tmpl.mk
