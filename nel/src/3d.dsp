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
CPP=xicl6.exe
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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "_WINDOWS" /D "__STL_DEBUG" /D "WIN32" /D "_DEBUG" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
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
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "_WINDOWS" /D "WIN32" /D "NDEBUG" /D "NL_RELEASE_DEBUG" /FR /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
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

SOURCE=.\3d\zone.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\zone.h
# End Source File
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
# Begin Source File

SOURCE=.\3d\viewport.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\viewport.h
# End Source File
# End Group
# Begin Group "Math"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3d\aabbox.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\aabbox.h
# End Source File
# Begin Source File

SOURCE=.\3d\bsphere.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\bsphere.h
# End Source File
# Begin Source File

SOURCE=.\3d\frustum.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\frustum.h
# End Source File
# Begin Source File

SOURCE=.\3d\polygon.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\polygon.h
# End Source File
# Begin Source File

SOURCE=.\3d\triangle.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\triangle.h
# End Source File
# Begin Source File

SOURCE=.\3d\triangle_ext.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\triangle_ext.h
# End Source File
# Begin Source File

SOURCE=.\3d\uv.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\uv.h
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
# End Group
# Begin Group "Textures"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\3d\bitmap.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\bitmap.h
# End Source File
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

SOURCE=.\3d\line.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\line.h
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
# End Target
# End Project
