CXX := g++
UNAME := $(shell uname)
CXXFLAGS := -Wall -Wextra -std=c++11 -O0 -g
ifeq ($(UNAME),Darwin)
	CXXFLAGS += -I/usr/local/Cellar/lua53/5.3.1_1/include/lua-5.3
	LDFLAGS := -L/usr/local/Cellar/lua53/5.3.1_1/lib/ -llua.5.3
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
