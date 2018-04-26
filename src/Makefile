# CEL(C Extension Library) Makefile
# Copyright (c) 2008 - 2018 Hu Jinya <hu_jinya@163.com>

CROSS =
CXX = $(CROSS)gcc
STRIP = $(CROSS)strip

CXXFLAGS = -Wall -Wno-deprecated -fPIC  -g  -I ../../cel/include
LDFLAGS = -L/usr/lib64/mysql
LIBS = -lpthread -lmysqlclient -lcrypt -lssl -lcrypto

A_TARGET = ../lib/libcel.a
SO_TARGET = ../lib/libcel.so
BIN_TARGET = 

MAJOR = 
MINOR = 
REVISION = 
BUILD = 
EXTRA = ""
VERSION_FILE=

SUB_DIRS = 
SRC_DIRS = _unix sys crypto net sql
SRC_EXTS = .c .cc .cpp .cxx 
OBJ_DIR = ../obj/src

include ../tmpl.mk
