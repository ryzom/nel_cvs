#
# $Id: Makefile.am,v 1.5 2001/02/06 09:27:05 valignat Exp $
#

install-exec-local:
	$(INSTALL_PROGRAM) -d $(includedir)/nel/3d
	$(INSTALL_PROGRAM) -d $(includedir)/nel/misc
	$(INSTALL_PROGRAM) -d $(includedir)/nel/net
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/agent
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/c
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/e
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/fuzzy
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/link
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/logic
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/nimat
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/pyserver
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/script
	$(INSTALL_PROGRAM) -d $(includedir)/nel/db
	$(INSTALL_PROGRAM) -d $(includedir)/nel/python
	$(INSTALL_PROGRAM) -d $(includedir)/nel/sys
	$(INSTALL_DATA) -m a=r `echo nel/*.h`             $(includedir)/nel
	$(INSTALL_DATA) -m a=r `echo nel/3d/*.h`          $(includedir)/nel/3d
	$(INSTALL_DATA) -m a=r `echo nel/misc/*.h`        $(includedir)/nel/misc
	$(INSTALL_DATA) -m a=r `echo nel/net/*.h`         $(includedir)/nel/net
	$(INSTALL_DATA) -m a=r `echo nel/ai/*.h`          $(includedir)/nel/ai/
	$(INSTALL_DATA) -m a=r `echo nel/ai/agent/*.h`    $(includedir)/nel/ai/agent
	$(INSTALL_DATA) -m a=r `echo nel/ai/c/*.h`        $(includedir)/nel/ai/c
	$(INSTALL_DATA) -m a=r `echo nel/ai/e/*.h`        $(includedir)/nel/ai/e
	$(INSTALL_DATA) -m a=r `echo nel/ai/fuzzy/*.h`    $(includedir)/nel/ai/fuzzy
	$(INSTALL_DATA) -m a=r `echo nel/ai/logic/*.h`    $(includedir)/nel/ai/logic
	$(INSTALL_DATA) -m a=r `echo nel/ai/nimat/*.h`    $(includedir)/nel/ai/nimat
	$(INSTALL_DATA) -m a=r `echo nel/ai/pyserver/*.h` $(includedir)/nel/ai/pyserver
	$(INSTALL_DATA) -m a=r `echo nel/ai/script/*.h`   $(includedir)/nel/ai/script


uninstall-local:
	rm -rf $(includedir)/nel


dist-hook:
	cp -pr $(srcdir)/nel $(distdir)
	rm -rf $(distdir)/CVS                   \
	       $(distdir)/nel/CVS               \
	       $(distdir)/nel/misc/CVS          \
	       $(distdir)/nel/net/CVS           \
	       $(distdir)/nel/3d/CVS            \
	       $(distdir)/nel/ai/CVS            \
	       $(distdir)/nel/ai/agent/CVS      \
	       $(distdir)/nel/ai/c/CVS          \
	       $(distdir)/nel/ai/e/CVS          \
	       $(distdir)/nel/ai/fuzzy/CVS      \
	       $(distdir)/nel/ai/link/CVS       \
	       $(distdir)/nel/ai/logic/CVS      \
	       $(distdir)/nel/ai/nimat/CVS      \
	       $(distdir)/nel/ai/pyserver/CVS   \
	       $(distdir)/nel/ai/script/CVS     \
	       $(distdir)/nel/db/CVS            \
	       $(distdir)/nel/python/CVS        \
	       $(distdir)/nel/sys/CVS


# End of Makefile.am

