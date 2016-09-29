CXX := g++
UNAME := $(shell uname)
CXXFLAGS := -Wall -Wextra -std=c++11 -O2
ifeq ($(UNAME),Darwin)
	LUA := $(shell brew --prefix lua53)
	CXXFLAGS += -I$(LUA)/include/lua-5.3
	LDFLAGS := -L$(LUA)/lib/ -llua.5.3
else  #linux?
	# The LUA_VER variable selects the appropriate lua version (5.2, 5.3, whatever)
	# The regular expression strips everything after the version number and passes
	# to sed. sed then removes the initial version and strips the sub-version number
	# (example:
	# $ lua -v
	# Lua 5.2.3 Copyright (C) 1994-2013 Lua.org, PUC-Rio
	# $ lua -v |grep -o -E 'Lua [^/\3]+'
	# Lua 5.2.3
	# $ lua -v | grep -o -E 'Lua [^/\3]+' | sed -e 's/Lua //' -e 's/\.//2g'
	# 5.2
	LUA_VER := $(shell lua -v | grep -o -E 'Lua [^/\3]+' | sed -e 's/Lua //' -e 's/\.//2g')
	LUA_LOC := /usr/include/lua$(LUA_VER)
	CXXFLAGS += -I$(LUA_LOC)
	LDFLAGS := -llua$(LUA_VER) -ldl
endif
BIN := main

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
