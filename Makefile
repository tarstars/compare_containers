CXX ?= g++
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra -Wpedantic

BIN_DIR := bin
TARGET := $(BIN_DIR)/benchmark
SOURCES := cpp/benchmark.cpp

all: $(TARGET)

$(TARGET): $(SOURCES)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CXXFLAGS) -o $@ $^

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: all run clean
