CC = gcc
SRCS = $(wildcard src/*.c)
TARGET = a.out

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
