#
# $Id: Makefile.am,v 1.5 2001/10/22 10:35:49 lecroart Exp $
#

MAINTAINERCLEANFILES = Makefile.in configure config.guess missing \
                       config.sub ltconfig aclocal.m4 config.h.in \
                       install-sh mkinstalldirs

SUBDIRS              = login_service          \
                       naming_service         \
                       admin_executor_service \
                       admin_service          \
                       welcome_service


dist-hook:
	cp -pr $(srcdir)/bootstrap $(distdir)


# End of Makefile.am

