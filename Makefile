CXX := g++
UNAME := $(shell uname)
CXXFLAGS := -Wall -Wextra -std=c++11 -O2
ifeq ($(UNAME),Darwin)
	LUA := $(shell brew --prefix lua@5.3)
	CXXFLAGS += -I$(LUA)/include/lua
	LDFLAGS := -L$(LUA)/lib/ -llua
else  #linux?
	# The LUA_VER variable selects the appropriate lua version (5.2, 5.3, whatever)
	# example:
	# $ lua -v
	# Lua 5.2.3 Copyright (C) 1994-2013 Lua.org, PUC-Rio
	# $ lua -v |grep -o -E '5.[0-9]'
	# 5.2
	LUA_VER := $(shell lua -v | grep -o -E '5.[0-9]')
	LUA_LOC := /usr/include/lua$(LUA_VER)
	CXXFLAGS += -I$(LUA_LOC)
	LDFLAGS := -llua$(LUA_VER) -ldl
endif
BIN := editor

src_files := $(wildcard *.cpp)
obj_files := $(patsubst %.cpp,%.o,$(src_files))

.PHONY: all clean remake

all: $(BIN)

clean:
	rm -f $(BIN) *.o

remake: clean all


$(BIN): $(obj_files)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp *.h
	$(CXX) $(CXXFLAGS) -c -o $@ $(filter %.cpp,$^)
