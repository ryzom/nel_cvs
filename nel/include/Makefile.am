#
# $Id: Makefile.am,v 1.12 2001/06/06 16:01:54 valignat Exp $
#

install-exec-local:
	$(INSTALL_PROGRAM) -d $(includedir)/nel/3d
	$(INSTALL_PROGRAM) -d $(includedir)/nel/3d/tmp
	$(INSTALL_PROGRAM) -d $(includedir)/nel/misc
	$(INSTALL_PROGRAM) -d $(includedir)/nel/net
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/agent
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/c
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/character
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/e
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/fuzzy
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/logic
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/nimat
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/pyserver
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/script
	$(INSTALL_PROGRAM) -d $(includedir)/nel/ai/tools
	$(INSTALL_PROGRAM) -d $(includedir)/nel/db
	$(INSTALL_PROGRAM) -d $(includedir)/nel/python
	$(INSTALL_PROGRAM) -d $(includedir)/nel/sys
	$(INSTALL_DATA) -m a=r `echo nel/*.h`             $(includedir)/nel
	$(INSTALL_DATA) -m a=r `echo nel/3d/*.h`          $(includedir)/nel/3d
	$(INSTALL_DATA) -m a=r `echo nel/3d/tmp/*.h`      $(includedir)/nel/3d/tmp
	$(INSTALL_DATA) -m a=r `echo nel/misc/*.h`        $(includedir)/nel/misc
	$(INSTALL_DATA) -m a=r `echo nel/net/*.h`         $(includedir)/nel/net
	$(INSTALL_DATA) -m a=r `echo nel/ai/*.h`          $(includedir)/nel/ai/
	$(INSTALL_DATA) -m a=r `echo nel/ai/agent/*.h`    $(includedir)/nel/ai/agent
	$(INSTALL_DATA) -m a=r `echo nel/ai/c/*.h`        $(includedir)/nel/ai/c
	$(INSTALL_DATA) -m a=r `echo nel/ai/character/*.h` $(includedir)/nel/ai/character
	$(INSTALL_DATA) -m a=r `echo nel/ai/e/*.h`        $(includedir)/nel/ai/e
	$(INSTALL_DATA) -m a=r `echo nel/ai/fuzzy/*.h`    $(includedir)/nel/ai/fuzzy
	$(INSTALL_DATA) -m a=r `echo nel/ai/logic/*.h`    $(includedir)/nel/ai/logic
	$(INSTALL_DATA) -m a=r `echo nel/ai/nimat/*.h`    $(includedir)/nel/ai/nimat
	$(INSTALL_DATA) -m a=r `echo nel/ai/pyserver/*.h` $(includedir)/nel/ai/pyserver
	$(INSTALL_DATA) -m a=r `echo nel/ai/script/*.h`   $(includedir)/nel/ai/script
	$(INSTALL_DATA) -m a=r `echo nel/ai/tools/*.h`    $(includedir)/nel/ai/tools


uninstall-local:
	rm -rf $(includedir)/nel


dist-hook:
	cp -pr $(srcdir)/nel $(distdir)
	cvslst=`ls -R $(distdir) | fgrep 'CVS:' | sed -e 's/:$$//' `; \
	-rm -rf $$cvslst


# End of Makefile.am

