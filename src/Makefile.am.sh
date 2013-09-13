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

if HAVE_SANCUS_NETLINK
AM_CFLAGS += \$(SANCUS_NETLINK_CFLAGS)
AM_LDFLAGS += \$(SANCUS_NETLINK_LIBS)

bin_PROGRAMS += \\
	$(list netlink/*.c | sed -e 's,\.c,,g' | sort -V)

endif
EOT
