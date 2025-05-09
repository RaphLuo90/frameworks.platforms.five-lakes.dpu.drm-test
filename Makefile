SRC_DIR := $(shell pwd)
LIB_DRMTEST := libdrmtest.a
DRMTEST := drmtest
MODETEST := vmodetest
BUILD_DIR := build
INSTALL_LIB_DRMTEST := $(BUILD_DIR)/$(LIB_DRMTEST)
INSTALL_DRMTEST := $(BUILD_DIR)/tools/$(DRMTEST)
INSTALL_MODETEST := $(BUILD_DIR)/tools/$(MODETEST)
CC=$(CROSS_COMPILE)gcc
AR=$(CROSS_COMPILE)ar

INC := -I$(SRC_DIR)/include
INC += -I$(SRC_DIR)/src
INC += -I$(DRM_DRIVER)/include/uapi

# in isar build, need to specify drm.h in default include path
INC += -I/usr/include/drm

LIBS := -ldrm
LIBS += -lvs_bo_helper
LIBS += -lm -ldl -rdynamic -lpthread

CFLAGS:= -fPIC -g -Wall -Werror -D_GNU_SOURCE -Wno-error=address -Wno-error=format-overflow

CONFIG_VERISILICON_MD5_CHECK ?= n
ifeq ($(VS_DRM_CONFIG_PCIE), 1)
	CONFIG_VERISILICON_MD5_CHECK = y
endif
ifeq ($(CONFIG_VERISILICON_MD5_CHECK), y)
	CFLAGS += -DCONFIG_VERISILICON_MD5_CHECK
	LIBS += -lm
	LIBS += -lcrypto
endif

CONFIG_VERISILICON_DEBUG ?= n
ifeq ($(CONFIG_VERISILICON_DEBUG), y)
	CFLAGS += -DCONFIG_VERISILICON_DEBUG
	vpath %.c src/option
	LIB_SRCS += ${wildcard src/option/*.c}
endif

#when enabled,the result of each frame and running time of each round
#  will be dumped.
CONFIG_VERISILICON_STREAM_DEBUG ?= n
ifeq ($(CONFIG_VERISILICON_STREAM_DEBUG), y)
	CFLAGS += -DCONFIG_VERISILICON_STREAM_DEBUG
endif

CONFIG_VERISILICON_DEC400_CONFORMANCE_TEST ?= n
ifeq ($(CONFIG_VERISILICON_DEC400_CONFORMANCE_TEST), y)
	CFLAGS += -DCONFIG_VERISILICON_DEC400_CONFORMANCE_TEST
endif

ifdef MAP_FUNCS_PATH
	CFLAGS += -DMAP_RELOCATION
	vpath %.c $(MAP_FUNCS_PATH)
	LIB_SRCS += ${notdir ${wildcard $(MAP_FUNCS_PATH)/*.c}}
endif

ifeq ($(CONFIG_VERISILICON_ARM64_QEMU), y)
        CFLAGS += -DCONFIG_VERISILICON_ARM64_QEMU
endif

vpath %.c src
LIB_SRCS += ${wildcard src/*.c}

vpath %.c src/util
LIB_SRCS += ${wildcard src/util/*.c}

vpath %.c src/map_funcs
LIB_SRCS += ${wildcard src/map_funcs/*.c}

vpath %.c src/json
LIB_SRCS += ${wildcard src/json/*.c}

LIB_OBJS := ${LIB_SRCS:%=$(BUILD_DIR)/%.o}

DRMTEST_OBJS := build/tools/drmtest.o
MODETEST_OBJS := build/tools/modetest.o

all: $(DRMTEST) $(MODETEST)

$(DRMTEST_OBJS):
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c tools/drmtest.c -o $@

$(MODETEST_OBJS):
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c tools/modetest.c -o $@

$(BUILD_DIR)/%.c.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c $< -o $@

$(INSTALL_LIB_DRMTEST): $(LIB_OBJS)
	$(AR) rcs $@ $^

$(INSTALL_DRMTEST):$(DRMTEST_OBJS) $(INSTALL_LIB_DRMTEST)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LIBS)

$(INSTALL_MODETEST): $(MODETEST_OBJS) $(INSTALL_LIB_DRMTEST)
	@mkdir -p $(dir $@)
	$(CC) -o $@ $^ $(LIBS)

$(DRMTEST): $(INSTALL_DRMTEST)
$(MODETEST): $(INSTALL_MODETEST)

clean:
	@rm -rf build

.PHONY:
	clean
