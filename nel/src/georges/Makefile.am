#
# $Id: Makefile.am,v 1.6 2002/06/03 10:00:49 lecroart Exp $
#

MAINTAINERCLEANFILES     = Makefile.in

lib_LTLIBRARIES          = libnelgeorges.la

libnelgeorges_la_SOURCES = form.cpp                        \
                           form_loader.cpp                 \
						   form_dfn.cpp                    \
						   form_elm.cpp                    \
                           stdgeorges.cpp                  \
                           stdgeorges.h                    \
						   header.cpp                      \
						   load_form.cpp                     \
						   sheet_id.cpp                      \
						   type.cpp

AM_CXXFLAGS         	 = -I$(top_srcdir)/src

libnelgeorges_la_LIBADD  = -lc -lpthread

libnelgeorges_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

