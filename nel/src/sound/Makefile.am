#
# $Id: Makefile.am,v 1.4 2002/02/18 12:57:44 lecroart Exp $
#

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS              = driver

lib_LTLIBRARIES      = libnelsnd.la

libnelsnd_la_SOURCES = ambiant_source.cpp                  \
                       ambiant_source.h                    \
                       audio_mixer_user.cpp                \
                       audio_mixer_user.h                  \
                       bounding_box.cpp                    \
                       bounding_box.h                      \
                       bounding_shape.h                    \
                       bounding_sphere.cpp                 \
                       bounding_sphere.h                   \
                       env_effect.cpp                      \
                       env_effect.h                        \
                       env_sound_user.cpp                  \
                       env_sound_user.h                    \
                       listener_user.cpp                   \
                       listener_user.h                     \
                       mixing_track.cpp                    \
                       mixing_track.h                      \
                       playable.h                          \
                       sound.cpp                           \
                       sound.h                             \
                       source_user.cpp                     \
                       source_user.h

AM_CXXFLAGS          = -I$(top_srcdir)/src

libnelsnd_la_LIBADD  = driver/libnelsnd_lowlevel.la

libnelsnd_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

