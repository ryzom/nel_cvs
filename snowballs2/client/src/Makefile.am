#
# $Id: Makefile.am,v 1.4 2001/07/27 14:33:06 valignat Exp $
#

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS         = snowballs

snowballs_SOURCES    = animation.cpp                       \
                       animation.h                         \
                       camera.cpp                          \
                       camera.h                            \
                       client.cpp                          \
                       client.h                            \
                       commands.cpp                        \
                       commands.h                          \
                       compass.cpp                         \
                       compass.h                           \
                       entities.cpp                        \
                       entities.h                          \
                       interface.cpp                       \
                       interface.h                         \
                       landscape.cpp                       \
                       landscape.h                         \
                       lens_flare.cpp                      \
                       lens_flare.h                        \
                       mouse_listener.cpp                  \
                       mouse_listener.h                    \
                       network.cpp                         \
                       network.h                           \
                       pacs.cpp                            \
                       pacs.h                              \
		       physics.cpp                         \
		       physics.h                           \
                       radar.cpp                           \
                       radar.h                             \
                       sound.cpp                           \
                       sound.h

snowballs_LDADD      = -lnelnet -lnelmisc -lnel3d -lnelpacs @FREETYPE_LIBS@

AM_CXXFLAGS          = @FREETYPE_CFLAGS@


# End of Makefile.am

