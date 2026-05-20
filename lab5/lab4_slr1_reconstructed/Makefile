CXX ?= g++
CXXFLAGS ?= -std=c++17 -O2 -Wall -Wextra
TARGET := slr1_reconstructed
SRC := src/main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET) --grammar examples/grammar_expr.txt --out-dir out

parse: $(TARGET)
	./$(TARGET) --grammar examples/grammar_expr.txt --out-dir out --parse "id + id * id"

clean:
	rm -f $(TARGET)
	rm -rf out/*

.PHONY: all run parse clean
