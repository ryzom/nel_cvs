#
# $Id: Makefile.am,v 1.2 2001/08/03 10:16:59 valignat Exp $
#

MAINTAINERCLEANFILES          = Makefile.in

SUBDIRS                       = openal .

noinst_LTLIBRARIES            = libnelsnd_lowlevel.la

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

libnelsnd_lowlevel_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

