#
# $Id: Makefile.am,v 1.5 2002/05/22 12:09:44 cado Exp $
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
						   type.cpp

AM_CXXFLAGS         	 = -I$(top_srcdir)/src

libnelgeorges_la_LIBADD  = -lc -lpthread

libnelgeorges_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

