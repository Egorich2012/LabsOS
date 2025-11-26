CC=gcc
CFLAGS=-Wall -Wextra -std=c99
TARGET=mychmod
SOURCES=mychmod.c

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

clean:
	rm -f $(TARGET)

.PHONY: clean
