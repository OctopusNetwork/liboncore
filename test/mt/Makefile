rootdir := $(shell pwd)/../../..
project := ubuntu-base

include $(rootdir)/build/common/inc.mk
ifeq ($(hostmaketype), windows)
rootdir := ../../..
endif

####
##  Application targets
####
DEP_HEADERS += $(incdir)/kktplatform $(incdir)/liboncore

apps = mt_core

mt_core_SRCS = $(rootdir)/liboncore/test/mt/main.c
mt_core_ARCHIVES = liboncore
mt_core_LDLIBS = -pthread -lm
mt_core_DISTSOLIBS =

include $(rootdir)/build/utils/app.mk
