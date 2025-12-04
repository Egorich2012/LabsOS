CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -O2
TARGET = mychmod
SOURCES = mychmod.c

all: $(TARGET)

$(TARGET): $(SOURCES)
  $(CC) $(CFLAGS) -o $@ $^

clean:
  rm -f $(TARGET)

.PHONY: all clean
