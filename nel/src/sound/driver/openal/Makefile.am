#
# $Id: Makefile.am,v 1.1 2001/08/01 08:45:06 valignat Exp $
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

AM_CXXFLAGS                  = -I$(top_srcdir)/src

libnel_drv_openal_la_LIBADD  = 

libnel_drv_openal_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

