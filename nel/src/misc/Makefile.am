#
# $Id: Makefile.am,v 1.17 2001/02/16 16:19:15 cado Exp $
#

MAINTAINERCLEANFILES  = Makefile.in

SUBDIRS               = config_file .

lib_LTLIBRARIES       = libnelmisc.la

libnelmisc_la_SOURCES = bit_set.cpp class_id.cpp class_registry.cpp \
                        debug.cpp displayer.cpp event_emitter.cpp \
                        event_listener.cpp event_server.cpp events.cpp \
                        file.cpp i18n.cpp log.cpp matrix.cpp mem_stream.cpp \
                        mutex.cpp path.cpp plane.cpp quat.cpp rgba.cpp \
                        smart_ptr.cpp stream.cpp task_manager.cpp \
                        time_nl.cpp vector.cpp vector_h.cpp vectord.cpp \
                        win_event_emitter.cpp win_thread.cpp rect.cpp \
                        system_info.cpp command.cpp p_thread.cpp common.cpp

libnelmisc_la_LIBADD  = config_file/libconfig.la -lpthread

libnelmisc_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

