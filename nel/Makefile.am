#
# $Id: Makefile.am,v 1.6 2001/05/02 17:11:33 valignat Exp $
#

MAINTAINERCLEANFILES = Makefile.in configure libtool config.guess \
                       config.sub ltconfig aclocal.m4 config.h.in \
                       install-sh missing mkinstalldirs ltmain.sh \
                       include/nelconfig.h include/nelconfig.h.in \
                       include/nel/nelconfig.h

DISTCLEANFILES       = include/stamp-h include/stamp-h.in 

SUBDIRS              = include src

bin_SCRIPTS          = nel-config

EXTRA_DIST           = nel.dsw

dist-hook:
	cp -pr $(srcdir)/bootstrap $(distdir)


# End of Makefile.am

