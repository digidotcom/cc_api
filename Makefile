# ***************************************************************************
# Copyright (c) 2011, 2012 Digi International Inc.,
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
CPP = g++
CC = gcc

# Target Platform
CCAPI_SOURCE_DIR = ./source
CCAPI_INCLUDE = ./include
CONNECTOR_DIR = $(CCAPI_SOURCE_DIR)/cc_ansic
CUSTOM_CONNECTOR_INCLUDE = $(CCAPI_SOURCE_DIR)/cc_ansic_custom_include
CONNECTOR_INCLUDE = $(CONNECTOR_DIR)/public/include
UNIT_TEST_INCLUDE = ./tests/unit_tests

TEST_DIR = ./tests/unit_tests
MOCKS_DIR = ./tests/mocks
CCIMP_SOURCE_DIR = ./tests/ccimp

# CFLAG Definition
CFLAGS += $(DFLAGS)
# Enable Compiler Warnings
CFLAGS += -Winit-self -Wpointer-arith
CFLAGS += -Wformat-security
CFLAGS += -Wformat-y2k -Wcast-align -Wformat-nonliteral
CFLAGS += -Wpadded -Wredundant-decls -Wvariadic-macros
CFLAGS += -Wall -Werror -Wextra -pedantic
CFLAGS += -Wno-error=padded -Wno-error=format-nonliteral -Wno-unused-function -Wno-missing-field-initializers

# Include POSIX and GNU features.
CFLAGS += -D_POSIX_C_SOURCE=200112L -D_GNU_SOURCE
# Include Public Header Files.
CFLAGS += -I$(UNIT_TEST_INCLUDE) -I$(MOCKS_DIR) -I$(CCAPI_INCLUDE) -I. -I$(CUSTOM_CONNECTOR_INCLUDE) -I$(CONNECTOR_INCLUDE) -I$(CCAPI_SOURCE_DIR)
CFLAGS += -g -O0

CCAPI_SOURCES = $(wildcard $(CCAPI_SOURCE_DIR)/*.c)
CCIMP_SOURCES = $(wildcard $(CCIMP_SOURCE_DIR)/*.c) 
TESTS_SOURCES := $(shell find $(TEST_DIR) -name '*.cpp')
MOCKS_SOURCES = $(wildcard $(MOCKS_DIR)/*.cpp)

CSRCS = $(CCAPI_SOURCES) $(CCIMP_SOURCES) 

CPPSRCS = $(wildcard ./*.cpp) $(TESTS_SOURCES) $(MOCKS_SOURCES)

# Libraries to Link
LIBS = -lc -lCppUTest -lCppUTestExt -lpthread -lrt

CCFLAGS += $(CFLAGS) -std=c89

# Generated Sample Executable Name.
EXEC_NAME = test

# since each of the samples shares private and platform files, do a clean each time we make
.PHONY:all
all: $(EXEC_NAME)

# Linking Flags.
LDFLAGS += $(DFLAGS) -Wl,-Map,$(EXEC_NAME).map,--sort-common

COBJS = $(CSRCS:.c=.o)
CPPOBJS = $(CPPSRCS:.cpp=.o)

test: $(COBJS) $(CPPOBJS)
	$(CPP) -DUNIT_TEST $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@
	./$@
	
.cpp.o:
	$(CPP) -DUNIT_TEST $(CFLAGS) -c $< -o $@

.c.o:
	$(CC) -DUNIT_TEST $(CCFLAGS) -c $< -o $@

.PHONY: clean
clean:
	-rm -f $(COBJS) $(CPPOBJS)
