CC = gcc
CFLAGS = -Wall -Wextra -std=c99

TARGET = compiler_lab

SRC = src/main.c src/grammar.c src/lr0.c src/scanner.c

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

run: $(TARGET)
	./$(TARGET)

scan: $(TARGET)
	./$(TARGET) --scan

clean:
	rm -f $(TARGET)
	rm -f data/lr0_output.json
	rm -f data/lr0_graph.dot
	rm -f data/token.txt