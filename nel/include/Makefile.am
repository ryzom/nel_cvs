#
# $Id: Makefile.am,v 1.1 2000/11/23 15:35:54 valignat Exp $
#

install-exec-local:
	$(INSTALL_PROGRAM) -d $(includedir)/nel/3d
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai
	$(INSTALL_PROGRAM) -d $(includedir)/nel/db
	$(INSTALL_PROGRAM) -d $(includedir)/nel/misc
	$(INSTALL_PROGRAM) -d $(includedir)/nel/net
	$(INSTALL_PROGRAM) -d $(includedir)/nel/python
	$(INSTALL_PROGRAM) -d $(includedir)/nel/sys
	$(INSTALL_DATA) -m a=r `echo nel/*.h`        $(includedir)/nel
	$(INSTALL_DATA) -m a=r `echo nel/3d/*.h`     $(includedir)/nel/3d
	$(INSTALL_DATA) -m a=r `echo nel/misc/*.h`   $(includedir)/nel/misc
	$(INSTALL_DATA) -m a=r `echo nel/net/*.h`    $(includedir)/nel/net


uninstall-local:
	rm -rf $(includedir)/nel


dist-hook:
	cp -pr $(srcdir)/nel $(distdir)
	rm -rf $(distdir)/CVS $(distdir)/nel/CVS $(distdir)/nel/3d/CVS \
	       $(distdir)/nel/ai/CVS $(distdir)/nel/db/CVS             \
	       $(distdir)/nel/misc/CVS $(distdir)/nel/net/CVS          \
	       $(distdir)/nel/python/CVS $(distdir)/nel/sys/CVS


# End of Makefile.am
