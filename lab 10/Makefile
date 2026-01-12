CC = gcc
CFLAGS = -Wall -Wextra -pthread
TARGET = rwlock

$(TARGET): rwlock.c
	$(CC) $(CFLAGS) -o $(TARGET) rwlock.c

clean:
	rm -f $(TARGET)

.PHONY: clean 
