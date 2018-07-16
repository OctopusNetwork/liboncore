rootdir := $(shell pwd)/../../..
project := ubuntu-base

include $(rootdir)/build/common/inc.mk
ifeq ($(hostmaketype), windows)
rootdir := ../../..
endif

TARGETSPEC = liboncore
TARGET_LIB = liboncore.$(staticlibsuffix)

DEP_ARCHIVES = libonplatform libonevent communication libontimer      \
               msgagent libonutils libonconfig penetrate encoding   \
               cJSON

MODULE = liboncore
MODULEVERSION = 0.0.1

include ../source.mk
include $(rootdir)/build/utils/merged-archive.$(hostmaketype).mk
