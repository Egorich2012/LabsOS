CC=gcc
CFLAGS=-Wall -Wextra -std=c99
TARGETS=pipe fifo_sender fifo_receiver

all: $(TARGETS)

pipe: pipe.c
	$(CC) $(CFLAGS) -o pipe pipe.c

fifo_sender: fifo_sender.c
	$(CC) $(CFLAGS) -o fifo_sender fifo_sender.c

fifo_receiver: fifo_receiver.c
	$(CC) $(CFLAGS) -o fifo_receiver fifo_receiver.c

clean:
	rm -f $(TARGETS) 

.PHONY: all clean
