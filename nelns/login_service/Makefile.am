#
# $Id: Makefile.am,v 1.5 2002/03/19 17:42:49 valignat Exp $
#

MAINTAINERCLEANFILES  = Makefile.in

EXTRA_DIST		       = login_service.cfg \
				 login_service.dsp

sbin_PROGRAMS         = login_service

login_service_SOURCES = connection_client.cpp              \
                        connection_client.h                \
                        connection_ws.cpp                  \
                        connection_ws.h                    \
                        login_service.cpp                  \
                        login_service.h

pkgsysconf_DATA	      = login_service.cfg

# End of Makefile.am

