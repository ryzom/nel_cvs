#
# $Id: Makefile.am,v 1.6 2002/08/20 12:47:31 lecroart Exp $
#

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS              = driver

lib_LTLIBRARIES      = libnelsnd.la

libnelsnd_la_SOURCES = ambiant_source.cpp                  \
                       ambiant_source.h                    \
                       audio_mixer_user.cpp                \
                       audio_mixer_user.h                  \
                       background_sound_manager.cpp        \
                       background_sound_manager.h          \
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
                       sample_bank.cpp                     \
                       sample_bank.h                       \
                       sound.cpp                           \
                       sound.h                             \
                       sound_bank.cpp                      \
                       sound_bank.h                        \
                       source_user.cpp                     \
                       source_user.h

AM_CXXFLAGS          = -I$(top_srcdir)/src

noinst_HEADERS	     = stdsound.h

libnelsnd_la_LIBADD  = driver/libnelsnd_lowlevel.la

libnelsnd_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

