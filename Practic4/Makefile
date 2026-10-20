CC = gcc
CFLAGS = -Wall -Wextra

SRC = $(wildcard src/*.c)

TARGETS = $(patsubst src/%.c,%,$(SRC))

all: $(TARGETS)

%: src/%.c
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -f $(TARGETS)

re: clean all

.PHONY: all clean re