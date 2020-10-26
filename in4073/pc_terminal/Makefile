
CC=gcc
CFLAGS = -g -Wall -lm
LLFLAGS = $(shell pkg-config --cflags --libs gtk+-3.0)
EXEC = ./pc-terminal

all:
	$(CC) $(CFLAGS) pc_terminal.c -o $(EXEC) $(LLFLAGS)

run: all
	$(EXEC)

