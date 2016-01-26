# Makefile
# Part of PUMPER, copyright (C) 2015 Alogfans.
#
# For testing purpose only, embedded with Google Test Fixture (gtest).

CC := gcc
CFLAGS := -g -Wall -Werror -std=c1x -rdynamic

CXX := g++
CXXFLAGS := -g -Wall -std=c++11 -rdynamic

EXECUTIVE_DIRECTORY := test
INTERMEDIATE_DIRECTORY := build
INCLUDING_DIRECTORY := include 
SOURCE_DIRECTORY := src
GTEST_LIBRARY_INCLUDE := gtest/include
GTEST_LIBRARY_DIRECTORY := gtest
GTEST_LIBRARY := gtest

TEST_SOURCES = $(notdir $(wildcard $(EXECUTIVE_DIRECTORY)/*.cpp))
FULL_SOURCES = $(wildcard $(SOURCE_DIRECTORY)/*.cpp)
SOURCES = $(notdir $(FULL_SOURCES))
OBJECTS = $(wildcard $(INTERMEDIATE_DIRECTORY)/*.o)

all: $(TEST_SOURCES)

$(TEST_SOURCES): $(SOURCES)
	@$(CXX) $(CXXFLAGS) $(EXECUTIVE_DIRECTORY)/$@ $(OBJECTS) -I$(INCLUDING_DIRECTORY) \
	-I$(GTEST_LIBRARY_INCLUDE) -L$(GTEST_LIBRARY_DIRECTORY) -lgtest -lpthread \
	-o $(INTERMEDIATE_DIRECTORY)/$(@:%.cpp=%)
	@echo [CXX] $(INTERMEDIATE_DIRECTORY)/$(@:%.cpp=%)

$(SOURCES): $(FULL_SOURCES)
	@$(CXX) $(CXXFLAGS) -c $(SOURCE_DIRECTORY)/$@ -I$(INCLUDING_DIRECTORY) \
	-o $(INTERMEDIATE_DIRECTORY)/$(@:%.cpp=%.o)
	@echo [CXX] $(INTERMEDIATE_DIRECTORY)/$(@:%.cpp=%.o)

clean:
	rm -f $(INTERMEDIATE_DIRECTORY)/*

.PHONY: all clean
