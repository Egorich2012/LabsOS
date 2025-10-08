CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGET = myls

all: $(TARGET)

$(TARGET): main.c
	$(CC) $(CFLAGS) -o $(TARGET) main.c

clean:
	rm -f $(TARGET)

.PHONY: all clean