CFLAGS=-std=c11 -g -fno-common

CC=gcc

SRCS=$(wildcard *.c)

OBJS=$(SRCS:.c=.o)

# rvcc depends on OBJS
rvcc: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# OBJS depends on rvcc.h
$(OBJS): rvcc.h

# test depends on rvcc
test: rvcc
	./test.sh

clean:
	rm -f rvcc $(OBJS) *.s tmp*