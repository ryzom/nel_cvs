#
# $Id: Makefile.am,v 1.4 2002/08/21 09:42:02 lecroart Exp $
#

MAINTAINERCLEANFILES          = Makefile.in

SUBDIRS                       = openal

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

# End of Makefile.am

