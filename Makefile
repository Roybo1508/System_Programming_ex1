# compiler and compilation flags
CC = gcc
CFLAGS = -Wall -Wextra -std=gnu11 -O2
TARGET = my_cp

all: $(TARGET)

# build the executable from the source file
$(TARGET): my_cp.c
	$(CC) $(CFLAGS) my_cp.c -o $(TARGET)

# delete the executable
clean:
	rm -f $(TARGET)

.PHONY: all clean
