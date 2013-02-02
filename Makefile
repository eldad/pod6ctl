VERSION=1.0

MAKEFLAGS += -rR --no-print-director

LIBS=-lasound
GCC=gcc -O2 -Wall -Werror -march=core2 -pipe ${LIBS} -DVERSION='"'${VERSION}'"' -std=c99

-include Makefile.cscope

%.o: %.c *.h
	${GCC} -c $< -o $@

.PHONY: all
all: pod6ctl cscope

dep_pod6ctl=pod6ctl.o bank.o sysex.o
pod6ctl: ${dep_pod6ctl} Makefile
	${GCC} ${dep_pod6ctl} -o $@

.PHONY: cscope
cscope:
	cscope -R -b ${CSCOPE_EXTRA}

.PHONY: clean
clean:
	rm *.o pod6ctl
