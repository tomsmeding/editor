CXX := g++
UNAME := $(shell uname)
CXXFLAGS := -Wall -Wextra -std=c++11 -O2
ifeq ($(UNAME),Darwin)
	LUA := $(wildcard /usr/local/Cellar/lua53/5.3.*)
	CXXFLAGS += -I$(LUA)/include/lua-5.3
	LDFLAGS := -L$(LUA)/lib/ -llua.5.3
else  #linux?
	CXXFLAGS +=
	LDFLAGS := -llua -ldl
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
