CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++11 -O2
BIN = main

src_files := $(wildcard *.cpp)
obj_files := $(patsubst %.cpp,%.o,$(src_files))

.PHONY: all clean remake run

all: $(BIN)

clean:
	rm -f $(BIN) *.o

remake: clean all

run: all
	./$(BIN)


$(BIN): $(obj_files)
	$(CXX) -o $@ $^

%.o: %.cpp *.h
	$(CXX) $(CXXFLAGS) -c -o $@ $(filter %.cpp,$^)
