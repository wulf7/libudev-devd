AC_PREREQ([2.64])

m4_define([libudev_version], [199])

AC_INIT([libudev],[libudev_version],[https://github.com/FreeBSDDesktop/libudev-devd],[libudev])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_AUX_DIR([ac-aux])
AC_CONFIG_MACRO_DIR([m4])
AC_CANONICAL_TARGET
AC_PROG_CC_C99

AM_INIT_AUTOMAKE([1.11 foreign no-dist-gzip dist-xz])
AM_SILENT_RULES([yes])

LT_PREREQ([2.2])
LT_INIT([disable-static])

AC_SUBST([PACKAGE_VERSION], [libudev_version])

AC_CHECK_HEADERS([libprocstat.h],
                 [AC_SEARCH_LIBS([procstat_open_sysctl], [procstat])],
                 [],
                 [[@%:@include <sys/param.h>]
                  [@%:@include <sys/queue.h>]
                  [@%:@include <sys/socket.h>]])
AC_CHECK_HEADERS([devinfo.h],
                 [AC_SEARCH_LIBS([devinfo_init], [devinfo])],
                 [],
                 [[@%:@include <devinfo.h>]])
AC_CHECK_HEADERS([linux/input.h])
AC_CHECK_FUNCS([pipe2 strchrnul])

AC_CONFIG_FILES([Makefile
		 libudev.pc
		])
AC_OUTPUT
