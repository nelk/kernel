CC = clang
CFLAGS = -Wall -Wextra -m32 -MMD -g -std=c99 -Qunused-arguments -pedantic
SOURCE = mem.c mem_test.c
OBJECTS = ${SOURCE:.c=.o}
DEPENDS = ${SOURCE:.c=.d}

EXECS = mem_test

all: test
test: ${EXECS}

mem_test: mem_test.o mem.o
	${CC} ${CFLAGS} -o $@ $^

clean:
	rm -rf ${OBJECTS} ${DEPENDS} ${EXECS} *.dSYM

-include ${DEPENDS}
