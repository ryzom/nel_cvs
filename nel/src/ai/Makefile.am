#
# $Id: Makefile.am,v 1.11 2002/02/19 11:20:57 valignat Exp $
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
                       tools

lib_LTLIBRARIES      = libnelai.la

libnelai_la_SOURCES  = agent_init.cpp \
                       static_def_init.cpp \
                       ai.cpp

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

