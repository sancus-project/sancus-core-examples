#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(sancus-core-examples, 0.0, amery@geeks.cl)
AC_CONFIG_HEADERS([src/config.h])

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])

AC_CONFIG_MACRO_DIR([m4])

LT_INIT
CWARNFLAGS="-Wall -Wextra -Winline -pedantic"
AC_SUBST(CWARNFLAGS)

# Checks for programs.
AC_PROG_CC_C99

# Checks for libraries.
PKG_CHECK_MODULES(SANCUS_CORE, [sancus-core])
AC_SUBST(SANCUS_CORE)
PKG_CHECK_MODULES(SANCUS_NETLINK, [sancus-netlink], [have_sancus_netlink=yes], [have_sancus_netlink=no])
AM_CONDITIONAL([HAVE_SANCUS_NETLINK], [test $have_sancus_netlink = yes])
AC_SUBST(SANCUS_NETLINK)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([Makefile
		 src/Makefile])
AC_OUTPUT
