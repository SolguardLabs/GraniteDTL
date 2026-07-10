CXX ?= c++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -Werror -O2
BUILD_DIR := build
TARGET := $(BUILD_DIR)/granitedtl
SOURCES := $(sort $(wildcard src/*.cpp))

.PHONY: all clean test ci list

all: $(TARGET)

$(TARGET): $(SOURCES) $(wildcard src/*.hpp)
	mkdir -p $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

test: $(TARGET)
	node --test --test-concurrency=1 "tests/node/*.test.js"

ci:
	node scripts/ci.mjs

list: $(TARGET)
	$(TARGET) --list

clean:
	rm -rf $(BUILD_DIR)
