#
# $Id: Makefile.am,v 1.7 2002/01/07 16:09:52 valignat Exp $
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
	cp -p  $(srcdir)/bootstrap $(distdir)
	cp -pR $(srcdir)/doc       $(distdir)
	cp -pR $(srcdir)/samples   $(distdir)
	cp -pR $(srcdir)/tools     $(distdir)


# End of Makefile.am

