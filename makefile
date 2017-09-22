CC=gcc
CFLAGS=-Wall -g --std=c99
OBJS=gbuild.o lexer.o project.o

all: gbuild

gbuild: $(OBJS)
	gcc $(OBJS) -o gbuild

clean:
	$(RM) *.o gbuild

.PHONY: all clean
