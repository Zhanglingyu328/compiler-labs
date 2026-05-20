CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra -Iinclude
SRC := $(wildcard src/*.cpp)
TARGET := lab5_semantic

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(SRC) -o $(TARGET)

run: $(TARGET)
	./$(TARGET) --batch examples -o output

one: $(TARGET)
	./$(TARGET) examples/1.src -o output

summary: $(TARGET)
	./$(TARGET) --batch examples -o output
	./scripts/summary.sh

clean:
	rm -f $(TARGET)
	rm -rf output/*

.PHONY: all run one summary clean