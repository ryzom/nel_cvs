#
# $Id: Makefile.am,v 1.5 2001/03/14 16:58:34 valignat Exp $
#

MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES      = libnelai.la

SUBDIRS              = agent c fuzzy logic nimat pyserver script .

libnelai_la_SOURCES  = ai.cpp static_def_init.cpp

libnelai_la_LIBADD   = agent/libagent.la \
                       c/libcommon.la \
                       fuzzy/libfuzzy.la \
                       logic/liblogic.la \
                       nimat/libnimat.la \
                       pyserver/libpyserver.la \
                       script/libscript.la

libnelai_la_LDFLAGS  = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

