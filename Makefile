CC = clang
CFLAGS = -Wall -Wextra -m32 -MMD -g -std=c99 -Qunused-arguments -pedantic

SOURCES = ${shell find . -name "*.c"}
TEST_SOURCES = ${shell find . -name "*_test.c"}
EXECS = ${TEST_SOURCES:.c=.out}
DEPENDS = ${SOURCES:.c=.d}
OBJECTS = ${SOURCES:.c=.o}
COMMON_OBJECTS = helpers.o


REPORT_SRC = report.tex
REPORT_OUTDIR = report_out
REPORT_TARG = report.pdf
LC = pdflatex

all: test
test: ${EXECS}

%_test.out: %_test.o %.o ${COMMON_OBJECTS}
	${CC} ${CFLAGS} $^ -o $@

clean:
	rm -rf ${OBJECTS} ${DEPENDS} ${EXECS} *.dSYM ${REPORT_TARG} ${REPORT_OUTDIR}

report: report.pdf

${REPORT_TARG}: ${REPORT_SRC}
	# ensure we don't use the partially completed build of a previous
	# compile
	rm -rf ${REPORT_OUTDIR};
	mkdir -p ${REPORT_OUTDIR};
	${LC} -output-directory ${REPORT_OUTDIR} ${REPORT_SRC};
	${LC} -output-directory ${REPORT_OUTDIR} ${REPORT_SRC};
	${LC} -output-directory ${REPORT_OUTDIR} ${REPORT_SRC};
	cp ${REPORT_OUTDIR}/${REPORT_TARG} ${REPORT_TARG};
	rm -r ${REPORT_OUTDIR};

-include ${DEPENDS}
