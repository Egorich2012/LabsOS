CC=gcc
CFLAGS=-Wall -Wextra -std=c99
TARGETS=sender receiver

all: $(TARGETS)

sender: sender.c
	$(CC) $(CFLAGS) -o sender sender.c

receiver: receiver.c
	$(CC) $(CFLAGS) -o receiver receiver.c

clean:
	rm -f $(TARGETS) 

.PHONY: all clean
