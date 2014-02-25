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
CONNECTOR_DIR = ./source/cc_ansic
CONNECTOR_INCLUDE = $(CONNECTOR_DIR)/public/include
UNIT_TEST_INCLUDE = ./tests/unit_tests
CCAPI_INCLUDE = ./include
CCAPI_SOURCE_DIR = ./source/
TEST_DIR = ./tests/unit_tests
MOCKS_DIR = ./tests/unit_tests/mocks

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
CFLAGS += -I$(UNIT_TEST_INCLUDE) -I$(CCAPI_INCLUDE) -I. -I$(CONNECTOR_INCLUDE) -I$(CCAPI_SOURCE_DIR)
CFLAGS += -g -O0

# Tell the environment we are doing UNIT_TESTING.
CFLAGS += -DUNIT_TESTING

# Target output to generate.
CSRCS = $(CCAPI_SOURCE_DIR)/ccapi_init.c $(CCAPI_SOURCE_DIR)/ccapi.c 
CPPSRCS = testrunner.cpp $(TEST_DIR)/ccapi_init_test.cpp $(TEST_DIR)/ccapi_config_test.cpp $(TEST_DIR)/ccapi_init_services_test.cpp $(MOCKS_DIR)/mock_ccimp_os.cpp $(MOCKS_DIR)/mock_connector_api.cpp

# Libraries to Link
LIBS = -lc -lCppUTest -lCppUTestExt -lpthread

CFLAGS += -DCONNECTOR_HAVE_STDINT_HEADER
CCFLAGS += $(CFLAGS) -std=c89

# Generated Sample Executable Name.
EXEC_NAME = test

# since each of the samples shares private and platform files, do a clean each time we make
.PHONY:all
all: clean $(EXEC_NAME)

# Linking Flags.
LDFLAGS += $(DFLAGS) -Wl,-Map,$(EXEC_NAME).map,--sort-common

COBJS = $(CSRCS:.c=.o)
CPPOBJS = $(CPPSRCS:.cpp=.o)

test: $(COBJS) $(CPPOBJS)
	$(CPP) -DUNIT_TEST $(CFLAGS) $(LDFLAGS) $^ $(LIBS) -o $@
	./$@

.cpp.o:
	$(CPP) -DUNIT_TEST $(CFLAGS) -c $^ -o $@

.c.o:
	$(CC) -DUNIT_TEST $(CCFLAGS) -c $^ -o $@

.PHONY: clean
clean:
	-rm -f $(EXEC_NAME) *.o *.map $(TEST_DIR)/*.o $(TEST_DIR)/*.map $(TEST_DIR)/*.map $(TEST_DIR)/*.o $(MOCKS_DIR)/*.o  $(MOCKS_DIR)/*.map $(CCAPI_SOURCE_DIR)/*.o $(CCAPI_SOURCE_DIR)/*.map
