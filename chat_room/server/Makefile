CC = gcc
CFLAG = -g -Wall -pthread

SRCS = $(wildcard src/*.c)
SRCS_CMD = $(wildcard src/commands/*.c)
SRCS_MSG = $(wildcard src/messages/*.c)
OBJS = $(patsubst src/%.c, obj/%.o, $(SRCS))
OBJS_CMD = $(patsubst src/commands/%.c, obj/%.o, $(SRCS_CMD))
OBJS_MSG = $(patsubst src/messages/%.c, obj/%.o, $(SRCS_MSG))

.PHONY: all clean

all: server

server: $(OBJS) $(OBJS_CMD) $(OBJS_MSG)
	$(CC) $^ -o $@ -lmysqlclient

obj/%.o: src/%.c
	$(CC) $(CFLAG) -c -o $@ $<

obj/%.o: src/commands/%.c
	$(CC) $(CFLAG) -c -o $@ $<

obj/%.o: src/messages/%.c
	$(CC) $(CFLAG) -c -o $@ $<

clean:
	rm -f ./obj/*.o server