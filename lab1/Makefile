CC = gcc
CFLAGS = -Wall -Wextra -std=c99
TARGETS = mycat mygrep

all: $(TARGETS)

mycat: main.c mycat.c mycat.h
	$(CC) $(CFLAGS) -DCOMPILE_CAT -o mycat main.c mycat.c

mygrep: main.c mygrep.c mygrep.h
	$(CC) $(CFLAGS) -DCOMPILE_GREP -o mygrep main.c mygrep.c

clean:
	rm -f $(TARGETS)

.PHONY: all clean