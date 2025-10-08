CC = g++
CFLAGS = -Wall -Wextra -std=c++17
TARGET = myls

all: $(TARGET)

$(TARGET): main.cpp
	$(CC) $(CFLAGS) -o $(TARGET) main.cpp

clean:
	rm -f $(TARGET)

.PHONY: all clean