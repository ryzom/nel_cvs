#
# $Id: Makefile.am,v 1.6 2001/07/27 17:25:40 valignat Exp $
#

MAINTAINERCLEANFILES         = Makefile.in

lib_LTLIBRARIES              = libnel_drv_opengl.la

libnel_drv_opengl_la_SOURCES = driver_opengl.cpp           \
                               driver_opengl.h             \
                               driver_opengl_extension.cpp \
                               driver_opengl_extension.h   \
                               driver_opengl_light.cpp     \
                               driver_opengl_material.cpp  \
                               driver_opengl_matrix.cpp    \
                               driver_opengl_texture.cpp   \
                               driver_opengl_vertex.cpp    \
                               unix_event_emitter.cpp      \
                               unix_event_emitter.h

AM_CXXFLAGS                  = -I$(top_srcdir)/src


# End of Makefile.am

