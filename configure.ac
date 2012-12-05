# Process this file with autoconf to produce a configure script.

# Initialization.
AC_PREREQ(2.61)
AC_INIT([darkpawns], [2.3])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([-Wall -Werror foreign no-dist])

# Basic configuration.
AC_CONFIG_SRCDIR([src/act.comm.c])
AC_CONFIG_HEADERS([src/config.h])

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
if test "$ac_cv_prog_cc_c99" = "no"; then
   AC_MSG_ERROR([cannot set compiler to c99 mode])
fi

AM_CONDITIONAL([C99], [test x$ac_cv_prog_cc_c99 != xno])

AC_PROG_RANLIB

# Checks for libraries.
AC_CHECK_LIB([m], [sqrt], [], 
        [AC_MSG_ERROR([cannot find libm])])
AC_CHECK_LIB([crypt], [crypt], [],
        [AC_MSG_WARN([cannot find libcrypt, passwords will be stored in plaintext])])
AC_CHECK_LIB([z], [deflate], [],
        [AC_MSG_ERROR([cannot find zlib])])

# Checks for library headers.
AC_CHECK_HEADERS([crypt.h arpa/telnet.h])

# Checks for types, structures, and compiler characteristics.
AC_TYPE_SIGNAL

# Checks for library functions.
AC_CHECK_FUNCS([crypt])

# Output
AC_CONFIG_FILES([Makefile src/Makefile])
AC_OUTPUT
