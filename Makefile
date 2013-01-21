CLANG = clang
GCC = gcc
CFLAGS = -Wall -Wextra -m32 -g -std=c99
SOURCE = mem.c mem_test.c
OBJECTS = ${SOURCE:.c=.o}
DEPENDS = ${SOURCE:.c=.d}
EXEC = test.out

# make -f clang test clean
# make -f gcc test clean

clang: ${SOURCE}
	${CLANG} ${CFLAGS} ${SOURCE} -o ${EXEC}

test:
	./${EXEC}

gcc:
	${GCC} ${CFLAGS} ${SOURCE} -o ${EXEC}

clean:
	rm -rf ${OBJECTS} ${DEPENDS} ${EXEC}

-include ${DEPENDS}	# reads the .d files and reruns dependencies