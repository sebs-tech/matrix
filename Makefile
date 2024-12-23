# Compiler and flags
CC = g++
CFLAGS = -Wall -Wextra -std=c++17 `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs` -lSDL2_ttf

# Target executable
TARGET = main
SRCS = src/main.cpp

# Build rules
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
