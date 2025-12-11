CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = archiver

$(TARGET): archiver.c
	$(CC) $(CFLAGS) -o $(TARGET) archiver.c

clean:
	rm -f $(TARGET)

.PHONY: clean 