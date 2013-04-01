CC = clang
CFLAGS = -Wall -Wextra -m32 -MMD -g -std=c99 -Qunused-arguments -pedantic

TEST_SOURCES = ${shell find . -name "*_test.c"}
SOURCES = ${TEST_SOURCES:_test.c=.c} ${TEST_SOURCES}
EXECS = ${TEST_SOURCES:.c=.out}
DEPENDS = ${SOURCES:.c=.d}
OBJECTS = ${SOURCES:.c=.o}

REPORT_SRC = report.tex
REPORT_OUTDIR = report_out
REPORT_TARG = report.pdf
LC = pdflatex

all: test
test: ${EXECS}

%_test.out: %_test.o %.o
	${CC} ${CFLAGS} $^ -o $@

clean:
	rm -rf ${OBJECTS} ${DEPENDS} ${EXECS} *.dSYM ${REPORT_TARG} ${REPORT_OUTDIR}

report: report.pdf

report.pdf: ${REPORT_SRC}
	mkdir -p ${REPORT_OUTDIR};
	${LC} -output-directory ${REPORT_OUTDIR} ${REPORT_SRC};
	${LC} -output-directory ${REPORT_OUTDIR} ${REPORT_SRC};
	${LC} -output-directory ${REPORT_OUTDIR} ${REPORT_SRC};
	cp ${REPORT_OUTDIR}/${REPORT_TARG} ${REPORT_TARG};
	rm -r ${REPORT_OUTDIR};

-include ${DEPENDS}
