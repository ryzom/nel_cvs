#
# $Id: Makefile.am,v 1.7 2001/03/27 08:13:02 chafik Exp $
#

MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES      = libnelai.la

SUBDIRS              = agent c character fuzzy logic nimat pyserver script .

libnelai_la_SOURCES  = ai.cpp static_def_init.cpp agent_init.cpp

libnelai_la_LIBADD   = agent/libagent.la \
                       character/libcharacter.la \
                       c/libcommon.la \
                       fuzzy/libfuzzy.la \
                       logic/liblogic.la \
                       nimat/libnimat.la \
                       pyserver/libpyserver.la \
                       script/libscript.la

libnelai_la_LDFLAGS  = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

