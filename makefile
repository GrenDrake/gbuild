CC=gcc
CFLAGS=-Wall -g --std=c99
OBJS=gbuild.o data.o lexer.o parser.o project.o
TARGET=gbuild

all: gbuild

$(TARGET): $(OBJS)
	gcc $(OBJS) -o $(TARGET)

clean:
	$(RM) *.o $(TARGET)

.PHONY: all clean
