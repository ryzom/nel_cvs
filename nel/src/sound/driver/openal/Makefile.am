#
# $Id: Makefile.am,v 1.5 2002/01/28 14:40:26 valignat Exp $
#

MAINTAINERCLEANFILES         = Makefile.in

lib_LTLIBRARIES              = libnel_drv_openal.la

libnel_drv_openal_la_SOURCES = buffer_al.cpp               \
                               buffer_al.h                 \
                               listener_al.cpp             \
                               listener_al.h               \
                               sound_driver_al.cpp         \
                               sound_driver_al.h           \
                               source_al.cpp               \
                               source_al.h

AM_CXXFLAGS                  = -I$(top_srcdir)/src @OPENAL_CFLAGS@

libnel_drv_openal_la_LIBADD  = @OPENAL_LIBS@

libnel_drv_openal_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

