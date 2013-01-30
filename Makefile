CC = clang
CFLAGS = -Wall -Wextra -m32 -MMD -g -std=c99 -Qunused-arguments -pedantic

TEST_SOURCES = ${shell find . -name "*_test.c"}
SOURCES = ${TEST_SOURCES:_test.c=.c} ${TEST_SOURCES}
EXECS = ${TEST_SOURCES:.c=.out}
DEPENDS = ${SOURCES:.c=.d}
OBJECTS = ${SOURCES:.c=.o}

all: test
test: ${EXECS}

%_test.out: %_test.o %.o
	${CC} ${CFLAGS} $^ -o $@

clean:
	rm -rf ${OBJECTS} ${DEPENDS} ${EXECS} *.dSYM

-include ${DEPENDS}
