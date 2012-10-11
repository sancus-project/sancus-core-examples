#!/bin/sh

list() {
	echo "$@" | tr ' ' '\n' | sort -V | tr '\n' '|' |
		sed -e 's,|$,,' -e 's,|, \\\n\t,g'
}

cd "${0%/*}"
cat <<EOT | tee Makefile.am
AM_CFLAGS = \$(CWARNFLAGS) \$(SANCUS_CORE_CFLAGS)
AM_LDFLAGS = \$(SANCUS_CORE_LIBS)

bin_PROGRAMS = \\
	$(list *.c | sed -e 's,\.c,,g' | sort -V)

EOT
