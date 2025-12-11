CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = mychmod

$(TARGET): mychmod.c
	$(CC) $(CFLAGS) -o $(TARGET) mychmod.c

clean:
	rm -f $(TARGET)

.PHONY: clean 