#
# $Id: Makefile.am,v 1.1 2001/08/01 08:45:06 valignat Exp $
#

MAINTAINERCLEANFILES          = Makefile.in

SUBDIRS                       = openal .

lib_LTLIBRARIES               = libnelsnd_lowlevel.la

libnelsnd_lowlevel_la_SOURCES = buffer.cpp                 \
                                buffer.h                   \
                                listener.cpp               \
                                listener.h                 \
                                loader.cpp                 \
                                loader.h                   \
                                sound_driver.cpp           \
                                sound_driver.h             \
                                source.cpp                 \
                                source.h

AM_CXXFLAGS                   = -I$(top_srcdir)/src

libnelsnd_lowlevel_la_LIBADD  = 

libnelsnd_lowlevel_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

