CFLAGS=-std=c11 -g -fno-common

CC=gcc

SRCS=$(wildcard *.c)

OBJS=$(SRCS:.c=.o)

rvcc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJS): rvcc.h

clean:
	rm -f rvcc $(OBJS)