#
# $Id: Makefile.am,v 1.37 2001/05/23 15:18:00 vizerie Exp $
#

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS              = driver .

lib_LTLIBRARIES      = libnel3d.la

libnel3d_la_SOURCES  = bezier_patch.cpp \
                       camera.cpp clip_trav.cpp computed_string.cpp \
                       driver.cpp dru.cpp event_mouse_listener.cpp \
                       font_generator.cpp font_manager.cpp frustum.cpp \
                       hrc_trav.cpp landscape.cpp landscape_model.cpp \
                       light_trav.cpp material.cpp mesh.cpp mini_col.cpp \
                       mot.cpp nelu.cpp patch.cpp patch_rdr_pass.cpp \
                       primitive_block.cpp register_3d.cpp \
                       render_trav.cpp scene.cpp shape.cpp tessellation.cpp \
                       text_context.cpp texture.cpp texture_file.cpp \
                       texture_font.cpp texture_mem.cpp texture_grouped.cpp tile_bank.cpp \
                       tile_element.cpp tile_far_bank.cpp transform.cpp \
                       transform_shape.cpp \
                       vertex_buffer.cpp viewport.cpp zone.cpp \
                       texture_far.cpp shader.cpp mrm_builder.cpp \
                       mrm_mesh.cpp tile_lumel.cpp texture_near.cpp \
                       zone_smoother.cpp zone_tgt_smoother.cpp \
                       landscape_profile.cpp tess_block.cpp tess_list.cpp \
                       zone_manager.cpp zone_search.cpp \
                       camera_user.cpp driver_user.cpp instance_user.cpp \
                       landscape_user.cpp material_user.cpp scene_user.cpp \
                       text_context_user.cpp texture_user.cpp \
                       transform_user.cpp driver_user2.cpp \
                       skeleton_weight.cpp animated_material.cpp \
                       animation_playlist.cpp play_list_manager.cpp \
                       anim_detail_trav.cpp channel_mixer.cpp \
                       animatable.cpp animated_value.cpp animation.cpp \
                       animation_set.cpp animation_time.cpp \
                       key.cpp track.cpp track_keyframer.cpp \
                       transformable.cpp mesh_instance.cpp \
                       scene_group.cpp skeleton_model.cpp bone.cpp \
                       skeleton_shape.cpp instance_group_user.cpp \
                       transformable_user.cpp skeleton_user.cpp \
                       bone_user.cpp instance_material_user.cpp \
                       shape_bank_user.cpp shape_bank.cpp height_map.cpp\
                       particle_system.cpp ps_color.cpp ps_emitter.cpp \
					   ps_float.cpp ps_force.cpp ps_int.cpp ps_located.cpp \
					   ps_particle.cpp ps_plane_basis_maker.cpp ps_util.cpp ps_zone.cpp \
					   particle_system_shape.cpp particle_system_model.cpp\
                       async_file_manager.cpp

AM_CXXFLAGS          = @FREETYPE_CFLAGS@

libnel3d_la_LIBADD   = @FREETYPE_LIBS@ -ldl -lGL

libnel3d_la_LDFLAGS  = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

