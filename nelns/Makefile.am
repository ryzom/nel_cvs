#
# $Id: Makefile.am,v 1.7 2002/04/10 07:33:07 lecroart Exp $
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

install-data-hook:
	mkdir -p ${DESTDIR}${pkglocalstatedir}
	mkdir -p ${DESTDIR}${logdir}
	mkdir -p ${DESTDIR}${pkgsysconfdir}
#
# Do not override existing config files
#
	for service in admin admin_executor login naming welcome ; \
	do \
		test -f ${DESTDIR}${pkgsysconfdir}/$${service}_service.cfg || cp ${srcdir}/$${service}_service/$${service}_service.cfg ${DESTDIR}${pkgsysconfdir} ; \
	done

uninstall-local:
	for service in admin admin_executor login naming welcome ; \
	do \
		rm -f ${DESTDIR}${pkgsysconfdir}/$${service}_service.cfg ; \
	done

# End of Makefile.am
