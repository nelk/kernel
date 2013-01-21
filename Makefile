CLANG = clang
GCC = gcc
CFLAGS = -Wall -Wextra -m32 -g -std=c99
SOURCE = mem.c mem_test.c
OBJECTS = ${SOURCE:.c=.o}
DEPENDS = ${SOURCE:.c=.d}
EXEC = test.out

# make -f clang run
# make -f gcc run

clang: ${SOURCE}
	${CLANG} ${CFLAGS} ${SOURCE} -o ${EXEC}

run:
	./${EXEC}

gcc:
	${GCC} ${CFLAGS} ${SOURCE} -o ${EXEC}

clean:
	rm -r ${OBJECTS} ${DEPENDS} ${EXEC}

-include ${DEPENDS}	# reads the .d files and reruns dependencies