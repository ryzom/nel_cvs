#
# $Id: Makefile.am,v 1.3 2002/08/20 11:37:36 lecroart Exp $
#

MAINTAINERCLEANFILES    = Makefile.in

noinst_LTLIBRARIES      = libconfig.la

YFLAGS = --defines=cf_gramatical.h -p cf

LFLAGS = -f -8 -Pcf -olex.yy.c

EXTRA_DIST = cf_gramatical.h

libconfig_la_SOURCES    = cf_lexical.lxx             \
                          cf_gramatical.yxx          \
                          config_file.cpp 

# End of Makefile.am
