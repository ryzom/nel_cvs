dnl ====================================================================
dnl Configuration script for NeLNS
dnl ====================================================================
dnl
dnl $Id: configure.ac,v 1.1 2004/01/15 19:28:55 lecroart Exp $
dnl 


dnl ====================================================================
dnl Process this file with autoconf to produce a configure script.
dnl ====================================================================

AC_INIT(admin_service/admin_service.cpp)

AC_CANONICAL_SYSTEM

MAJOR_VERSION=0
MINOR_VERSION=3
PICO_VERSION=20020923

AM_INIT_AUTOMAKE(nelns, $MAJOR_VERSION.$MINOR_VERSION.$PICO_VERSION)

AM_CONFIG_HEADER(config.h)


dnl The following hack should ensure that configure doesn't add optimizing
dnl or debugging flags to CFLAGS or CXXFLAGS
CFLAGS="$CFLAGS "
CXXFLAGS="$CXXFLAGS "


dnl ====================================================================
dnl Checks for programs.
dnl ====================================================================

AC_PROG_MAKE_SET

AC_PROG_CC
AC_PROG_CXX

AM_SANITY_CHECK

AC_PROG_INSTALL

AM_PATH_CCACHE


AM_PATH_MYSQL(3.23.40,,AC_MSG_ERROR([MySQL 3.23.40 or higher must be installed]))

AC_DEFINE(HAVE_MYSQL)

CFLAGS="$CFLAGS $MYSQL_CFLAGS"
CXXFLAGS="$CXXFLAGS $MYSQL_CFLAGS"

LIBS="$LIBS $MYSQL_LIBS"


dnl ====================================================================
dnl Configure Settings
dnl ====================================================================

AC_LANG_CPLUSPLUS


dnl ====================================================================
dnl Debug/optimized compilation mode
dnl ====================================================================

AM_NEL_DEBUG

dnl Set the gcc specific warning level
if test "X$CC" = "Xgcc";
then
    if test "$with_debug" != "no"
    then
	#
	# When debugging variables are declared for the sole purpose of
	# inspecting their content with a debugger. They are not used
	# in the code itself and this is legitimate, hence the -Wno-unused
	#
        CXXFLAGS="$CXXFLAGS -Wall -Wno-multichar -Wno-unused"
    else
        CXXFLAGS="$CXXFLAGS -Wall -Wno-multichar"
    fi
fi

dnl ====================================================================
dnl Checks for typedefs, structures, and compiler characteristics.
dnl ====================================================================

dnl Add the define _REENTRANT for a correct use of the threads
if test "X$CC" = "Xgcc";
then
    CXXFLAGS="$CXXFLAGS -D_REENTRANT"
fi


dnl ====================================================================
dnl Checks for header files.
dnl ====================================================================

AC_STDC_HEADERS
AC_CHECK_HEADERS(fcntl.h unistd.h)


dnl ====================================================================
dnl Checks compilation accelerator
dnl ====================================================================

AM_PATH_CCACHE

dnl ====================================================================
dnl Checks for libraries.
dnl ====================================================================

AC_CHECK_LIB(crypt, crypt)

dnl ====================================================================
dnl STLPort

AM_PATH_STLPORT


dnl ====================================================================
dnl NeL

dnl         misc   net    3d    pacs  sound  ai    georges
AM_PATH_NEL("yes", "yes", "no", "no", "no",  "no", "no")

dnl ====================================================================
dnl Checks for library functions.
dnl ====================================================================

dnl ====================================================================
dnl nelns configuration and logs file location
dnl ====================================================================

test "x$prefix" = xNONE && prefix=$ac_default_prefix

AC_ARG_WITH(sysconfdir,
    [  --with-sysconfdir[=DIR]
                          use DIR instead of the default sysconfdir/nelns],
    [pkgsysconfdir=$withval],
    [pkgsysconfdir='${sysconfdir}/nelns'])
AC_SUBST(pkgsysconfdir)

eval eval NELNS_CONFIG=\"$pkgsysconfdir/\"
AC_DEFINE_UNQUOTED(NELNS_CONFIG, "$NELNS_CONFIG")

AC_ARG_WITH(localstatedir,
    [  --with-localstatedir[=DIR]
                          use DIR instead of the default localstatedir/game/nelns],
    [pkglocalstatedir=$withval],
    [pkglocalstatedir=${localstatedir}/game/nelns])
AC_SUBST(pkglocalstatedir)

eval eval NELNS_STATE=\"$pkglocalstatedir/\"
AC_DEFINE_UNQUOTED(NELNS_STATE, "$NELNS_STATE")

AC_ARG_WITH(logdir,
    [  --with-logdir[=DIR]
                          use DIR instead of the default localstatedir/log/nelns],
    [logdir=$withval],
    [logdir='${localstatedir}/log/nelns'])
AC_SUBST(logdir)

eval eval NELNS_LOGS=\"$logdir/\"
AC_DEFINE_UNQUOTED(NELNS_LOGS, "$NELNS_LOGS")

dnl ====================================================================
dnl Output files to generate.
dnl ====================================================================

AC_OUTPUT( Makefile                        \
           login_service/Makefile          \
           naming_service/Makefile         \
           admin_executor_service/Makefile \
           admin_service/Makefile          \
           welcome_service/Makefile        \
)


dnl End of configure.in

