#
# $Id: Makefile.am,v 1.3 2001/05/03 13:19:12 lecroart Exp $
#

MAINTAINERCLEANFILES = Makefile.in configure config.guess missing \
                       config.sub ltconfig aclocal.m4 config.h.in \
                       install-sh mkinstalldirs

SUBDIRS              = login_service          \
                       log_service            \
                       naming_service         \
                       time_service           \
                       admin_executor_service \
                       admin_service          \
                       welcome_service        \
                       agent_service


dist-hook:
	cp -pr $(srcdir)/bootstrap $(distdir)


# End of Makefile.am

