# Microsoft Developer Studio Project File - Name="3d" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=3d - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "3d.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "3d.mak" CFG="3d - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "3d - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "3d - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "3d - Win32 ReleaseDebug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "3d - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "3d___Win32_Release"
# PROP BASE Intermediate_Dir "3d___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "_MBCS" /D "_LIB" /D "WIN32" /D "NDEBUG" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/nl3d.lib"

!ELSEIF  "$(CFG)" == "3d - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "3d___Win32_Debug"
# PROP BASE Intermediate_Dir "3d___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "_MBCS" /D "_LIB" /D "WIN32" /D "_DEBUG" /D "__STL_DEBUG" /FR /YX /FD /GZ /Zm200 /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/nl3d_debug.lib"

!ELSEIF  "$(CFG)" == "3d - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "3d___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "3d___Win32_ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "_MBCS" /D "_LIB" /D "WIN32" /D "NDEBUG" /D "NL_RELEASE_DEBUG" /FR /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"../lib/nl3d.lib"
# ADD LIB32 /nologo /out:"../lib/nl3d_rd.lib"

!ENDIF 

# Begin Target

# Name "3d - Win32 Release"
# Name "3d - Win32 Debug"
# Name "3d - Win32 ReleaseDebug"
# Begin Group "Traversals"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3d\anim_detail_trav.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\anim_detail_trav.h
# End Source File
# Begin Source File

SOURCE=.\3d\clip_trav.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\clip_trav.h
# End Source File
# Begin Source File

SOURCE=.\3d\hrc_trav.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\hrc_trav.h
# End Source File
# Begin Source File

SOURCE=.\3d\light_trav.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\light_trav.h
# End Source File
# Begin Source File

SOURCE=.\3d\render_trav.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\render_trav.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\trav_scene.h
# End Source File
# End Group
# Begin Group "Models"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3d\camera.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\camera.h
# End Source File
# Begin Source File

SOURCE=.\3d\landscape_model.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\landscape_model.h
# End Source File
# Begin Source File

SOURCE=.\3d\mesh_instance.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\mesh_instance.h
# End Source File
# Begin Source File

SOURCE=.\3d\skeleton_model.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\skeleton_model.h
# End Source File
# Begin Source File

SOURCE=.\3d\transform.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\transform.h
# End Source File
# Begin Source File

SOURCE=.\3d\transform_shape.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\transform_shape.h
# End Source File
# End Group
# Begin Group "Landscape"

# PROP Default_Filter ""
# Begin Group "ZoneManager"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3d\zone_manager.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\zone_manager.h
# End Source File
# Begin Source File

SOURCE=.\3d\zone_search.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\zone_search.h
# End Source File
# End Group
# Begin Group "Tools"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3d\zone_smoother.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\zone_smoother.h
# End Source File
# Begin Source File

SOURCE=.\3d\zone_tgt_smoother.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\zone_tgt_smoother.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\3d\bezier_patch.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\bezier_patch.h
# End Source File
# Begin Source File

SOURCE=.\3d\landscape.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\landscape.h
# End Source File
# Begin Source File

SOURCE=.\3d\landscape_profile.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\landscape_profile.h
# End Source File
# Begin Source File

SOURCE=.\3d\patch.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\patch.h
# End Source File
# Begin Source File

SOURCE=.\3d\patch_rdr_pass.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\patch_rdr_pass.h
# End Source File
# Begin Source File

SOURCE=.\3d\tess_block.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tess_block.h
# End Source File
# Begin Source File

SOURCE=.\3d\tess_list.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tess_list.h
# End Source File
# Begin Source File

SOURCE=.\3d\tessellation.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tessellation.h
# End Source File
# Begin Source File

SOURCE=.\3d\texture_far.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\texture_far.h
# End Source File
# Begin Source File

SOURCE=.\3d\texture_near.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\texture_near.h
# End Source File
# Begin Source File

SOURCE=.\3d\tile_color.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tile_color.h
# End Source File
# Begin Source File

SOURCE=.\3d\tile_element.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tile_element.h
# End Source File
# Begin Source File

SOURCE=.\3d\tile_lumel.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tile_lumel.h
# End Source File
# Begin Source File

SOURCE=.\3d\zone.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\zone.h
# End Source File
# End Group
# Begin Group "Driver"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3d\driver.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\driver.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\driver_material_inline.h
# End Source File
# Begin Source File

SOURCE=.\3d\dru.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\dru.h
# End Source File
# Begin Source File

SOURCE=.\3d\light.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\light.h
# End Source File
# Begin Source File

SOURCE=.\3d\material.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\material.h
# End Source File
# Begin Source File

SOURCE=.\3d\primitive_block.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\primitive_block.h
# End Source File
# Begin Source File

SOURCE=.\3d\shader.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\shader.h
# End Source File
# Begin Source File

SOURCE=.\3d\texture.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\texture.h
# End Source File
# Begin Source File

SOURCE=.\3d\vertex_buffer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\vertex_buffer.h
# End Source File
# End Group
# Begin Group "Font"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3d\computed_string.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\computed_string.h
# End Source File
# Begin Source File

SOURCE=.\3d\font_generator.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\font_generator.h
# End Source File
# Begin Source File

SOURCE=.\3d\font_manager.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\font_manager.h
# End Source File
# Begin Source File

SOURCE=.\3d\text_context.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\text_context.h
# End Source File
# End Group
# Begin Group "Container"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3d\ptr_set.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\ptr_set.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\quad_grid.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\quad_tree.h
# End Source File
# End Group
# Begin Group "Tiles"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3d\tile_bank.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tile_bank.h
# End Source File
# Begin Source File

SOURCE=.\3d\tile_far_bank.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tile_far_bank.h
# End Source File
# End Group
# Begin Group "Shapes"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3d\mesh.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\mesh.h
# End Source File
# Begin Source File

SOURCE=.\3d\shape.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\shape.h
# End Source File
# Begin Source File

SOURCE=R:\code\nel\src\3d\shape_bank.cpp
# End Source File
# Begin Source File

SOURCE=R:\code\nel\include\nel\3d\shape_bank.h
# End Source File
# Begin Source File

SOURCE=.\3d\skeleton_shape.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\skeleton_shape.h
# End Source File
# End Group
# Begin Group "Textures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3d\texture_file.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\texture_file.h
# End Source File
# Begin Source File

SOURCE=.\3d\texture_font.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\texture_font.h
# End Source File
# Begin Source File

SOURCE=.\3d\texture_mem.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\texture_mem.h
# End Source File
# End Group
# Begin Group "Mrm"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3d\mrm_builder.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\mrm_builder.h
# End Source File
# Begin Source File

SOURCE=.\3d\mrm_mesh.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\mrm_mesh.h
# End Source File
# End Group
# Begin Group "Animation"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3d\animatable.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\animatable.h
# End Source File
# Begin Source File

SOURCE=.\3d\animated_material.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\animated_material.h
# End Source File
# Begin Source File

SOURCE=.\3d\animated_value.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\animated_value.h
# End Source File
# Begin Source File

SOURCE=.\3d\animation.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\animation.h
# End Source File
# Begin Source File

SOURCE=.\3d\animation_playlist.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\animation_playlist.h
# End Source File
# Begin Source File

SOURCE=.\3d\animation_set.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\animation_set.h
# End Source File
# Begin Source File

SOURCE=.\3d\animation_time.cpp
# End Source File
# Begin Source File

SOURCE=.\3d\bone.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\bone.h
# End Source File
# Begin Source File

SOURCE=.\3d\channel_mixer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\channel_mixer.h
# End Source File
# Begin Source File

SOURCE=.\3d\key.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\key.h
# End Source File
# Begin Source File

SOURCE=.\3d\play_list_manager.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\play_list_manager.h
# End Source File
# Begin Source File

SOURCE=.\3d\skeleton_weight.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\skeleton_weight.h
# End Source File
# Begin Source File

SOURCE=.\3d\track.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\track.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\track_bezier.h
# End Source File
# Begin Source File

SOURCE=.\3d\track_keyframer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\track_keyframer.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\track_tcb.h
# End Source File
# Begin Source File

SOURCE=.\3d\transformable.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\transformable.h
# End Source File
# End Group
# Begin Group "UserImp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3d\animation_set_user.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\animation_set_user.h
# End Source File
# Begin Source File

SOURCE=.\3d\bone_user.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\bone_user.h
# End Source File
# Begin Source File

SOURCE=.\3d\camera_user.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\camera_user.h
# End Source File
# Begin Source File

SOURCE=.\3d\driver_user.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\driver_user.h
# End Source File
# Begin Source File

SOURCE=.\3d\driver_user2.cpp
# End Source File
# Begin Source File

SOURCE=.\3d\frustum.cpp
# End Source File
# Begin Source File

SOURCE=.\3d\instance_group_user.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\instance_group_user.h
# End Source File
# Begin Source File

SOURCE=.\3d\instance_user.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\instance_user.h
# End Source File
# Begin Source File

SOURCE=.\3d\landscape_user.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\landscape_user.h
# End Source File
# Begin Source File

SOURCE=.\3d\material_user.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\material_user.h
# End Source File
# Begin Source File

SOURCE=.\3d\play_list_manager_user.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\play_list_manager_user.h
# End Source File
# Begin Source File

SOURCE=.\3d\play_list_user.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\play_list_user.h
# End Source File
# Begin Source File

SOURCE=.\3d\scene_user.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\scene_user.h
# End Source File
# Begin Source File

SOURCE=.\3d\skeleton_user.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\skeleton_user.h
# End Source File
# Begin Source File

SOURCE=.\3d\text_context_user.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\text_context_user.h
# End Source File
# Begin Source File

SOURCE=.\3d\texture_user.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\texture_user.h
# End Source File
# Begin Source File

SOURCE=.\3d\transform_user.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\transform_user.h
# End Source File
# Begin Source File

SOURCE=.\3d\transformable_user.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\transformable_user.h
# End Source File
# Begin Source File

SOURCE=.\3d\viewport.cpp
# End Source File
# End Group
# Begin Group "User"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\nel\3d\tmp\animation_time.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\frustum.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\u_animation_set.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\u_bone.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\u_camera.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\u_driver.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\u_instance.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\u_instance_group.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\u_landscape.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\u_material.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\u_play_list.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\u_play_list_manager.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\u_scene.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\u_skeleton.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\u_text_context.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\u_texture.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\u_transform.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\u_transformable.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tmp\viewport.h
# End Source File
# End Group
# Begin Group "Makefile"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3d\Makefile.am
# End Source File
# End Group
# Begin Group "sfx"

# PROP Default_Filter ""
# End Group
# Begin Source File

SOURCE=.\3d\event_mouse_listener.cpp

!IF  "$(CFG)" == "3d - Win32 Release"

!ELSEIF  "$(CFG)" == "3d - Win32 Debug"

# SUBTRACT CPP /YX

!ELSEIF  "$(CFG)" == "3d - Win32 ReleaseDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\event_mouse_listener.h
# End Source File
# Begin Source File

SOURCE=.\3d\mini_col.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\mini_col.h
# End Source File
# Begin Source File

SOURCE=.\3d\mot.cpp

!IF  "$(CFG)" == "3d - Win32 Release"

# ADD CPP /YX

!ELSEIF  "$(CFG)" == "3d - Win32 Debug"

!ELSEIF  "$(CFG)" == "3d - Win32 ReleaseDebug"

# ADD BASE CPP /YX
# ADD CPP /YX

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\mot.h
# End Source File
# Begin Source File

SOURCE=.\3d\nelu.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\nelu.h
# End Source File
# Begin Source File

SOURCE=.\3d\register_3d.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\register_3d.h
# End Source File
# Begin Source File

SOURCE=.\3d\scene.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\scene.h
# End Source File
# Begin Source File

SOURCE=.\3d\scene_group.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\scene_group.h
# End Source File
# End Target
# End Project
