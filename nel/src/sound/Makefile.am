#
# $Id: Makefile.am,v 1.7 2002/11/04 15:40:43 boucher Exp $
#

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS              = driver

lib_LTLIBRARIES      = libnelsnd.la

libnelsnd_la_SOURCES = audio_mixer_user.cpp                \
                       audio_mixer_user.h                  \
                       background_source.cpp               \
                       background_source.h                 \
					   complex_source.cpp                  \
					   complex_source.h                    \
                       listener_user.cpp                   \
                       listener_user.h                     \
					   source_common.cpp                   \
					   source_common.h                     \
                       source_user.cpp                     \
                       source_user.h                       \
					   background_sound.cpp				   \            
					   background_sound.h				   \            
					   complex_sound.cpp                   \
					   complex_sound.h                     \
					   somple_sound.cpp                    \
					   simple_sound.h                      \
                       sound.cpp                           \
                       sound.h                             \
					   sound_anim_manager.cpp              \
					   sound_anim_manager.h                \
					   sound_anim_marker.cpp               \
					   sound_anim_marker.h                 \
					   sound_animation.cpp                 \
					   sound_animation.h                   \
					   async_file_manager_sound.cpp        \
					   async_file_manager_sound.h          \
                       background_sound_manager.cpp        \
                       background_sound_manager.h          \
                       mixing_track.cpp                    \
                       mixing_track.h                      \
                       playable.h                          \
                       sample_bank.cpp                     \
                       sample_bank.h                       \
                       sound_bank.cpp                      \
                       sound_bank.h                        

AM_CXXFLAGS          = -I$(top_srcdir)/src

noinst_HEADERS	     = stdsound.h

libnelsnd_la_LIBADD  = driver/libnelsnd_lowlevel.la

libnelsnd_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

