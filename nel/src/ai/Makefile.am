#
# $Id: Makefile.am,v 1.9 2001/07/27 15:40:25 valignat Exp $
#

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS              = agent                               \
                       c                                   \
                       character                           \
                       fuzzy                               \
                       logic                               \
                       nimat                               \
                       pyserver                            \
                       script                              \
                       tools                               \
                       .

lib_LTLIBRARIES      = libnelai.la

libnelai_la_SOURCES  = ai.cpp                              \
                       static_def_init.cpp                 \
		       agent_init.cpp

libnelai_la_LIBADD   = agent/libagent.la                   \
                       character/libcharacter.la           \
                       c/libcommon.la                      \
                       fuzzy/libfuzzy.la                   \
                       logic/liblogic.la                   \
                       nimat/libnimat.la                   \
                       pyserver/libpyserver.la             \
                       script/libscript.la                 \
                       tools/libtools.la

libnelai_la_LDFLAGS  = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

