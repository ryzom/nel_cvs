#
# $Id: Makefile.am,v 1.5 2005/04/04 09:45:06 cado Exp $
#

MAINTAINERCLEANFILES          = Makefile.in

SUBDIRS                       = fmod

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

