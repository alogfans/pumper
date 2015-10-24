# Makefile
# Part of PUMPER, copyright (C) 2015 Alogfans.
#
# For testing purpose only, embedded with Google Test Fixture (gtest).

CC := gcc
CFLAGS := -g -Wall -Werror -Wextra -std=c1x

CXX := g++
CXXFLAGS := -g -Wall -Werror -Wextra -std=c++11

EXECUTIVES := testBacktrace testHashTable

INTERMEDIATE_DIRECTORY := ../build
INCLUDING_DIRECTORIES := . ../include ../src ../gtest/include
GTEST_LIBRARY_DIRECTORY := ../gtest
GTEST_LIBRARY := gtest

all: $(EXECUTIVES)

$(EXECUTIVES):
	$(CXX) -c $(SRCS) -I$(INCLUDE) -L$(LIB_DIR)

clean:
	

.PHONY: all clean