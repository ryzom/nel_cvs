#
# $Id: Makefile.am,v 1.7 2000/12/18 15:27:41 lecroart Exp $
#

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS              = driver .

lib_LTLIBRARIES      = libnel3d.la

libnel3d_la_SOURCES  = aabbox.cpp bezier_patch.cpp bitmap.cpp bsphere.cpp \
                       camera.cpp clip_trav.cpp computed_string.cpp \
                       driver.cpp dru.cpp event_mouse_listener.cpp \
                       font_generator.cpp font_manager.cpp hrc_trav.cpp \
                       landscape.cpp landscape_model.cpp light_trav.cpp \
                       material.cpp mot.cpp patch.cpp patch_rdr_pass.cpp \
                       primitive_block.cpp render_trav.cpp scene.cpp \
                       nelu.cpp tessellation.cpp texture.cpp \
                       tile_bank.cpp tile_element.cpp transform.cpp \
                       vertex_buffer.cpp viewport.cpp zone.cpp frustum.cpp \
                       polygon.cpp texture_font.cpp texture_file.cpp \
                       texture_mem.cpp register_3d.cpp transform_shape.cpp \
                       mesh.cpp shape.cpp

libnel3d_la_LIBADD    = @FREETYPE_LIBS@

AM_CXXFLAGS           = @FREETYPE_CFLAGS@

# End of Makefile.am
