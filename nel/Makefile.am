#
# $Id: Makefile.am,v 1.4 2000/11/23 16:50:07 valignat Exp $
#

MAINTAINERCLEANFILES = Makefile.in configure libtool config.guess \
                       config.sub ltconfig aclocal.m4 config.h.in \
                       install-sh missing mkinstalldirs ltmain.sh \
                       include/nelconfig.h include/nelconfig.h.in \
                       include/nel/nelconfig.h

DISTCLEANFILES       = include/stamp-h include/stamp-h.in 

SUBDIRS              = include src

dist-hook:
	cp -pr $(srcdir)/bootstrap $(distdir)

# End of Makefile.am
