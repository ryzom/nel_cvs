#
# $Id: Makefile.am,v 1.21 2001/05/09 09:43:30 berenguier Exp $
#

MAINTAINERCLEANFILES  = Makefile.in

SUBDIRS               = config_file .

lib_LTLIBRARIES       = libnelmisc.la

libnelmisc_la_SOURCES = aabbox.cpp bit_set.cpp bitmap.cpp bsphere.cpp \
                        buf_fifo.cpp class_id.cpp class_registry.cpp \
                        command.cpp common.cpp debug.cpp displayer.cpp \
                        event_emitter.cpp event_listener.cpp event_server.cpp \
                        events.cpp file.cpp geom_ext.cpp i18n.cpp line.cpp \
                        log.cpp matrix.cpp mem_stream.cpp mutex.cpp \
                        p_thread.cpp path.cpp plane.cpp polygon.cpp \
                        quad.cpp quat.cpp rect.cpp rgba.cpp smart_ptr.cpp \
                        stream.cpp string_id_array.cpp system_info.cpp \
                        task_manager.cpp time_nl.cpp triangle.cpp uv.cpp \
                        vector.cpp vector_h.cpp vectord.cpp vector_2f.cpp

libnelmisc_la_LIBADD  = config_file/libconfig.la -lpthread

libnelmisc_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

