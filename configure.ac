
AC_INIT([simplefb],[0.0.4])
AC_CONFIG_HEADERS([config.h])
AC_PREREQ(2.59)
AC_COPYRIGHT([Copyright (C) 2015 STC Metrotek])
AC_REVISION($Revision: 1.23.2.1 $)

# Make sure we got a valid path to the sources.
AC_CONFIG_SRCDIR([simplefb.c])

AC_CONFIG_MACRO_DIR([m4])

AM_PROG_CC_C_O

AM_INIT_AUTOMAKE([-Wall])

AM_MAINTAINER_MODE([enable])

#AC_CHECK_LIB(libX11)

# SUBPROJ support
AM_CONDITIONAL([SUBPROJ],[test "x$SUBPROJ" = "xyes"])

AC_CONFIG_FILES([
	Makefile
	])
AC_OUTPUT
