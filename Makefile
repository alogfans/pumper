# Makefile
# Part of PUMPER, copyright (C) 2015 Alogfans.
#
# For testing purpose only, embedded with Google Test Fixture (gtest).

CC := gcc
CFLAGS := -g -Wall -Werror -Wextra -std=c1x -rdynamic

CXX := g++
CXXFLAGS := -g -Wall -Werror -Wextra -std=c++11 -rdynamic

EXECUTIVE_DIRECTORY := ./test
INTERMEDIATE_DIRECTORY := ./build
INCLUDING_DIRECTORY := ./include 
SOURCE_DIRECTORY := ./src
GTEST_LIBRARY_INCLUDE := ./gtest/include
GTEST_LIBRARY_DIRECTORY := ./gtest
GTEST_LIBRARY := gtest

TEST_SOURCES = $(notdir $(wildcard $(EXECUTIVE_DIRECTORY)/*.cpp))
EXECUTIVES = $(TEST_SOURCES:%.cpp=%)
SOURCES = $(wildcard $(SOURCE_DIRECTORY)/*.cpp)

all: $(TEST_SOURCES)

$(TEST_SOURCES):
	$(CXX) $(CXXFLAGS) $(EXECUTIVE_DIRECTORY)/$@ $(SOURCES) -I$(INCLUDING_DIRECTORY) -o $(@:%.cpp=$(INTERMEDIATE_DIRECTORY)/%)

$(EXECUTIVES):
	$(INTERMEDIATE_DIRECTORY)/$@

clean:
	rm -rf $(INTERMEDIATE_DIRECTORY)
	mkdir $(INTERMEDIATE_DIRECTORY)

.PHONY: all clean
