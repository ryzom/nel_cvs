#
# $Id: Makefile.am,v 1.3 2001/07/27 15:51:10 valignat Exp $
#

MAINTAINERCLEANFILES  = Makefile.in

bin_PROGRAMS          = login_service

login_service_SOURCES = connection_client.cpp              \
                        connection_client.h                \
                        connection_ws.cpp                  \
                        connection_ws.h                    \
                        login_service.cpp                  \
                        login_service.h

login_service_LDADD   = -lnelnet -lnelmisc -lcrypt


# End of Makefile.am

