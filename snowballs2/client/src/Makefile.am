#
# $Id: Makefile.am,v 1.1 2001/07/11 15:17:40 legros Exp $
#

MAINTAINERCLEANFILES = Makefile.in

bin_PROGRAMS         = snowballs

snowballs_SOURCES    = character_interface.cpp client.cpp \
                       language_interface.cpp \
                       login_interface.cpp move_listener.cpp \
                       pb_message_box.cpp player_view.cpp \
                       shards_list_interface.cpp sight.cpp \
                       lens_flare.cpp

snowballs_LDADD      = -lnelnet -lnelmisc -lnel3d @FREETYPE_LIBS@

AM_CXXFLAGS          = @FREETYPE_CFLAGS@


# End of Makefile.am
