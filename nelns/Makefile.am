#
# $Id: Makefile.am,v 1.1 2001/04/18 13:54:25 valignat Exp $
#

MAINTAINERCLEANFILES = Makefile.in configure config.guess missing \
                       config.sub ltconfig aclocal.m4 config.h.in \
                       install-sh mkinstalldirs

SUBDIRS              = login_service          \
                       log_service            \
                       naming_service         \
                       time_service           \
                       agent_service          \
                       admin_executor_service


dist-hook:
	cp -pr $(srcdir)/bootstrap $(distdir)


# End of Makefile.am

