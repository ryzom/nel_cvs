#
# $Id: Makefile.am,v 1.2 2000/11/24 17:17:23 valignat Exp $
#

MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES      = libnel3d.la

libnel3d_la_SOURCES  = aabbox.cpp bezier_patch.cpp bitmap.cpp bsphere.cpp \
                      camera.cpp clip_trav.cpp driver.cpp font_generator.cpp \
                      font_manager.cpp hrc_trav.cpp landscape.cpp \
                      landscape_model.cpp light_trav.cpp material.cpp \
                      mot.cpp patch.cpp primitive_block.cpp render_trav.cpp \
                      scene.cpp scene_ut.cpp tessellation.cpp texture.cpp \
                      tile_bank.cpp transform.cpp vertex_buffer.cpp zone.cpp


all-local:
	cp -f ./.libs/* $(top_srcdir)/lib


# End of Makefile.am

