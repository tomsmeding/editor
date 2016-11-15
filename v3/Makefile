TERMIO_PREFIX = $(HOME)/prefix
BIN = editor


CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -fwrapv -I$(TERMIO_PREFIX)/include
LDFLAGS = -L$(TERMIO_PREFIX)/lib -ltermio
DEBUG = 1
ifneq ($(DEBUG),)
	CXXFLAGS += -g
else
	CXXFLAGS += -O2
endif

UNAME_VALUE = $(shell uname)


.PHONY: all clean remake

all: $(BIN)

clean:
	rm -rf $(BIN) *.o *.dSYM

remake: clean all


$(BIN): $(patsubst %.cpp,%.o,$(wildcard *.cpp))
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp $(wildcard *.h)
	$(CXX) $(CXXFLAGS) -c -o $@ $<
