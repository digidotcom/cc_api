# ***************************************************************************
# Copyright (c) 2013 Digi International Inc.,
# All rights not expressly granted are reserved.
# 
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this file,
# You can obtain one at http://mozilla.org/MPL/2.0/.
# 
# Digi International Inc. 11001 Bren Road East, Minnetonka, MN 55343
#
# ***************************************************************************
# Use GNU C Compiler
CC = gcc

# Location of Private Connector Code.
CC_PRIVATE_DIR=../../../source/cc_ansic/private
CCAPI_PRIVATE_DIR=../../../source
# Locate of Public Include Header Files.
CC_PUBLIC_HEADER_DIR=../../../source/cc_ansic/public/include
CCAPI_PUBLIC_HEADER_DIR=../../../include
# Location of Platform Src Code.
PLATFORM_DIR=../../ccimp

# Resolves where to find Source files.
vpath $(CC_PRIVATE_DIR)/%.c
vpath $(PLATFORM_DIR)/%.c

# CFLAG Definition
CFLAGS += $(DFLAGS)
# Enable Compiler Warnings
CFLAGS += -Winit-self -Wbad-function-cast -Wpointer-arith
CFLAGS += -Wmissing-parameter-type -Wstrict-prototypes -Wformat-security 
CFLAGS += -Wformat-y2k -Wold-style-definition -Wcast-align -Wformat-nonliteral 
CFLAGS += -Wpadded -Wredundant-decls -Wvariadic-macros
CFLAGS += -Wall -Werror -Wextra -pedantic
CFLAGS += -Wno-error=padded -Wno-error=format-nonliteral -Wno-unused-function -Wno-missing-field-initializers 
# Use ANSIC 99
CFLAGS +=-std=c99 
# Include POSIX and GNU features.
CFLAGS += -D_POSIX_C_SOURCE=200112L -D_GNU_SOURCE 
# Include Public Header Files.
CFLAGS += -I. -I$(CC_PUBLIC_HEADER_DIR) -I$(CCAPI_PUBLIC_HEADER_DIR)
# Include Platform Header Files.
CFLAGS += -I$(PLATFORM_DIR)


ifeq ($(DEBUG),true)
CFLAGS += -g -O0 -DDEBUG
else
# Optimize for Size
CFLAGS += -Os
endif

# Target output to generate.
APP_SRCS = main.c
CC_PRIVATE_SRCS = $(CC_PRIVATE_DIR)/connector_api.c
CCAPI_PRIVATE_SRCS = $(CCAPI_PRIVATE_DIR)/ccapi.c $(CCAPI_PRIVATE_DIR)/ccapi_init.c $(CCAPI_PRIVATE_DIR)/ccapi_logging.c
PLATFORM_SRCS = $(PLATFORM_DIR)/ccimp_os.c $(PLATFORM_DIR)/ccimp_logging.c $(PLATFORM_DIR)/ccimp_hal.c $(PLATFORM_DIR)/ccimp_network_tcp.c $(PLATFORM_DIR)/dns_helper.c
SRCS = $(APP_SRCS) $(PLATFORM_SRCS) $(CC_PRIVATE_SRCS) $(CCAPI_PRIVATE_SRCS)

# Libraries to Link
LIBS = -lc -lz -lpthread -lrt 

# Generated Sample Executable Name.
SAMPLE = ccapi

# since each of the samples shares private and platform files, do a clean each time we make
.PHONY:all
all: clean $(SAMPLE)

# Linking Flags.
LDFLAGS += $(DFLAGS) -Wl,-Map,$(SAMPLE).map,--sort-common

OBJS = $(SRCS:.c=.o)

$(SAMPLE): $(OBJS)
	$(CC) $(LDFLAGS) $^ $(LIBS) -o $@
	


.PHONY: clean
clean:
	-rm -f $(SAMPLE) $(OBJS) $(SAMPLE).map
