#
# $Id: Makefile.am,v 1.4 2002/01/07 16:09:52 valignat Exp $
#

MAINTAINERCLEANFILES  = Makefile.in

bin_PROGRAMS          = login_service

login_service_SOURCES = connection_client.cpp              \
                        connection_client.h                \
                        connection_ws.cpp                  \
                        connection_ws.h                    \
                        login_service.cpp                  \
                        login_service.h

login_service_LDADD   = -lnelmisc -lnelnet -lcrypt


# End of Makefile.am

