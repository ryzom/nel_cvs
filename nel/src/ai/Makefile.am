#
# $Id: Makefile.am,v 1.3 2001/01/12 09:41:38 valignat Exp $
#

MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES      = libnelai.la

SUBDIRS              = agent c fuzzy logic pyserver script .

libnelai_la_SOURCES  = ia.cpp static_def_init.cpp

libnelai_la_LIBADD   = agent/libagent.la \
                       c/libcommon.la \
                       fuzzy/libfuzzy.la \
                       logic/liblogic.la \
                       pyserver/libpyserver.la \
                       script/libscript.la

# End of Makefile.am

