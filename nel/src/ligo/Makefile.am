#
# $Id: Makefile.am,v 1.1 2002/09/10 17:26:04 coutelas Exp $
#

MAINTAINERCLEANFILES     = Makefile.in

lib_LTLIBRARIES          = libnelligo.la

libnelligo_la_SOURCES = ligo_config.cpp                        \
                        ligo_error.cpp                 \
			ligo_material.cpp                    \
			primitive.cpp                    \
                        transition.cpp                  \
                        zone_bank.h                    \
			zone_edge.cpp                      \
			zone_region.cpp                   \
			zone_template.cpp

AM_CXXFLAGS         	 = -I$(top_srcdir)/src

libnelligo_la_LIBADD  = -lc -lpthread

libnelligo_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

