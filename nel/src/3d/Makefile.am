#
# $Id: Makefile.am,v 1.16 2001/02/05 16:55:25 corvazier Exp $
#

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS              = driver .

lib_LTLIBRARIES      = libnel3d.la

libnel3d_la_SOURCES  = aabbox.cpp bezier_patch.cpp bitmap.cpp bsphere.cpp \
                       camera.cpp clip_trav.cpp computed_string.cpp \
                       driver.cpp dru.cpp event_mouse_listener.cpp \
                       font_generator.cpp font_manager.cpp frustum.cpp \
                       hrc_trav.cpp landscape.cpp landscape_model.cpp \
                       light_trav.cpp material.cpp mesh.cpp mini_col.cpp \
                       mot.cpp nelu.cpp patch.cpp patch_rdr_pass.cpp \
                       polygon.cpp primitive_block.cpp register_3d.cpp \
                       render_trav.cpp scene.cpp shape.cpp tessellation.cpp \
                       text_context.cpp texture.cpp texture_file.cpp \
                       texture_font.cpp texture_mem.cpp tile_bank.cpp \
                       tile_element.cpp tile_far_bank.cpp transform.cpp \
                       transform_shape.cpp triangle.cpp uv.cpp \
                       vertex_buffer.cpp viewport.cpp zone.cpp \
                       texture_far.cpp shader.cpp mrm_builder.cpp \
                       mrm_mesh.cpp tile_lumel.cpp texture_near.cpp \
                       zone_smoother.cpp zone_tgt_smoother.cpp \
					   animatable.cpp animated_value.cpp animation.cpp \
					   channel.cpp channel_mixer.cpp key.cpp track.cpp \
					   animation_time.cpp 

AM_CXXFLAGS          = @FREETYPE_CFLAGS@

libnel3d_la_LIBADD   = @FREETYPE_LIBS@ -ldl -lGL

libnel3d_la_LDFLAGS  = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

