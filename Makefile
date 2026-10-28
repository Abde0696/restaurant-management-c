CC      ?= cc
CFLAGS  = -std=c11 -Wall -Wextra -pedantic -Iinclude -MMD -MP
SRC     = $(wildcard src/*.c)
OBJ     = $(SRC:src/%.c=build/%.o)
TARGET  = restaurant

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build:
	mkdir -p build

clean:
	rm -rf build $(TARGET)

-include $(OBJ:.o=.d)

.PHONY: all clean
