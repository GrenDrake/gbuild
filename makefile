CC=gcc
CFLAGS=-Wall -g --std=c99 `pkg-config --cflags check`
OBJS=gbuild.o data.o lexer.o parser.o project.o
TARGET=gbuild

all: gbuild

test: test/lexerTest
	test/lexerTest

$(TARGET): $(OBJS)
	gcc $(OBJS) -o $(TARGET)

test/lexerTest: test/lexer.o lexer.o data.o
	gcc test/lexer.o lexer.o data.o `pkg-config --libs check` -o test/lexerTest

clean:
	$(RM) *.o $(TARGET)

.PHONY: all clean test
