#
# $Id: Makefile.am,v 1.2 2001/05/02 12:36:39 lecroart Exp $
#

MAINTAINERCLEANFILES = Makefile.in configure config.guess missing \
                       config.sub ltconfig aclocal.m4 config.h.in \
                       install-sh mkinstalldirs

SUBDIRS              = login_service          \
                       log_service            \
                       naming_service         \
                       time_service           \
                       agent_service          \
                       admin_executor_service \
                       admin_service          \
                       welcome_service        \


dist-hook:
	cp -pr $(srcdir)/bootstrap $(distdir)


# End of Makefile.am

