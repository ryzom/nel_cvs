#
# $Id: Makefile.am,v 1.6 2002/03/19 17:42:48 valignat Exp $
#

MAINTAINERCLEANFILES = Makefile.in configure config.guess missing \
                       config.sub ltconfig aclocal.m4 config.h.in \
                       install-sh mkinstalldirs 

EXTRA_DIST	     = acconfig.h services.dsw bootstrap doc

SUBDIRS              = login_service          \
                       naming_service         \
                       admin_executor_service \
                       admin_service          \
                       welcome_service

dist-hook:
	find $(distdir) -name CVS -print | xargs rm -fr

# End of Makefile.am

