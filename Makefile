CC = gcc
CFLAGS = -Wall -g -I.
TARGET = compiled/omnish
SRCS = main.c string.c tokenizer.c parser.c executor.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p compiled
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

test: all
	./tests/test_shell.sh

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
