CC = gcc
CFLAGS = -Wall -Wextra -std=c99

TARGET_CAT = mycat
TARGET_GREP = mygrep

$(TARGET_CAT): main.c mycat.c mycat.h
	$(CC) $(CFLAGS) -o $(TARGET_CAT) main.c mycat.c
 
$(TARGET_GREP): main.c mygrep.c mygrep.h
	$(CC) $(CFLAGS) -o $(TARGET_GREP) main.c mygrep.c

all: $(TARGET_CAT) $(TARGET_GREP)

clean:
	rm -f $(TARGET_CAT) $(TARGET_GREP)

rebuild: clean all

.PHONY: all clean rebuild