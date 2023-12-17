CC = gcc
CFLAG = -g -Wall -pthread

SRCS = $(wildcard src/*.c)
OBJS = $(patsubst src/%.c, obj/%.o, $(SRCS))

.PHONY: all clean

all: server

server: $(OBJS)
	$(CC) $^ -o $@ -lmysqlclient

obj/%.o: src/%.c
	$(CC) $(CFLAG) -c -o $@ $<

clean:
	rm -f ./obj/*.o server