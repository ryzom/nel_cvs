#
# $Id: Makefile.am,v 1.3 2000/11/24 17:17:23 valignat Exp $
#

MAINTAINERCLEANFILES  = Makefile.in

SUBDIRS               = config_file .

lib_LTLIBRARIES       = libnelmisc.la

libnelmisc_la_SOURCES = bit_set.cpp class_id.cpp class_registry.cpp \
                        debug.cpp displayer.cpp event_emitter.cpp \
                        event_listener.cpp event_server.cpp events.cpp \
                        file.cpp i18n.cpp log.cpp matrix.cpp plane.cpp \
                        quat.cpp rgba.cpp smart_ptr.cpp stream.cpp \
                        time_nl.cpp vector.cpp vector_h.cpp vectord.cpp

libnelmisc_la_LIBADD  = config_file/libconfig.la


all-local:
	cp -f ./.libs/* $(top_srcdir)/lib


# End of Makefile.am

