# Microsoft Developer Studio Project File - Name="pacs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=pacs - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pacs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pacs.mak" CFG="pacs - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pacs - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pacs - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "pacs - Win32 ReleaseDebug" (based on "Win32 (x86) Static Library")
!MESSAGE "pacs - Win32 DebugFast" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pacs - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../obj/Release/pacs"
# PROP Intermediate_Dir "../obj/Release/pacs"
# PROP Target_Dir ""
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdpacs.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x40c
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /nologo /out:"../lib/nlpacs_r.lib"

!ELSEIF  "$(CFG)" == "pacs - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../obj/Debug/pacs"
# PROP Intermediate_Dir "../obj/Debug/pacs"
# PROP Target_Dir ""
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /D "_LIB" /D "__STL_DEBUG" /D "_MBCS" /D "WIN32" /D "_DEBUG" /Yu"stdpacs.h" /FD /GZ /c
# SUBTRACT CPP /Gf /Gy /Fr
# ADD BASE RSC /l 0x40c
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /nologo /out:"../lib/nlpacs_d.lib"

!ELSEIF  "$(CFG)" == "pacs - Win32 ReleaseDebug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../obj/ReleaseDebug/pacs"
# PROP Intermediate_Dir "../obj/ReleaseDebug/pacs"
# PROP Target_Dir ""
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NL_RELEASE_DEBUG" /Yu"stdpacs.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x40c
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /nologo /out:"../lib/nlpacs_rd.lib"

!ELSEIF  "$(CFG)" == "pacs - Win32 DebugFast"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../obj/DebugFast/pacs"
# PROP Intermediate_Dir "../obj/DebugFast/pacs"
# PROP Target_Dir ""
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /Ob1 /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "NL_DEBUG_FAST" /Yu"stdpacs.h" /FD /GZ /c
# SUBTRACT CPP /Gf /Gy /Fr
# ADD BASE RSC /l 0x40c
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /nologo /out:"../lib/nlpacs_df.lib"

!ENDIF 

# Begin Target

# Name "pacs - Win32 Release"
# Name "pacs - Win32 Debug"
# Name "pacs - Win32 ReleaseDebug"
# Name "pacs - Win32 DebugFast"
# Begin Group "Static Collisions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pacs\chain_quad.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\chain_quad.h
# End Source File
# Begin Source File

SOURCE=.\pacs\collision_mesh_build.h
# End Source File
# Begin Source File

SOURCE=.\pacs\collision_surface_temp.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\collision_surface_temp.h
# End Source File
# Begin Source File

SOURCE=.\pacs\edge_collide.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\edge_collide.h
# End Source File
# Begin Source File

SOURCE=.\pacs\edge_quad.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\edge_quad.h
# End Source File
# Begin Source File

SOURCE=.\pacs\exterior_mesh.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\exterior_mesh.h
# End Source File
# Begin Source File

SOURCE=.\pacs\face_grid.h
# End Source File
# End Group
# Begin Group "Dynamic Collisions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pacs\collision_callback.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\collision_callback.h
# End Source File
# Begin Source File

SOURCE=.\pacs\collision_desc.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\collision_desc.h
# End Source File
# Begin Source File

SOURCE=.\pacs\collision_ot.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\collision_ot.h
# End Source File
# Begin Source File

SOURCE=.\pacs\move_cell.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\move_cell.h
# End Source File
# Begin Source File

SOURCE=.\pacs\move_container.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\move_container.h
# End Source File
# Begin Source File

SOURCE=.\pacs\move_container_inline.h
# End Source File
# Begin Source File

SOURCE=.\pacs\move_element.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\move_element.h
# End Source File
# Begin Source File

SOURCE=.\pacs\move_element_inline.h
# End Source File
# Begin Source File

SOURCE=.\pacs\move_primitive.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\move_primitive.h
# End Source File
# Begin Source File

SOURCE=.\pacs\primitive_block.h
# End Source File
# Begin Source File

SOURCE=.\pacs\primitive_block_pacs.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\primitive_world_image.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\primitive_world_image.h
# End Source File
# End Group
# Begin Group "Pathfinding"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pacs\chain.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\chain.h
# End Source File
# Begin Source File

SOURCE=.\pacs\global_retriever.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\global_retriever.h
# End Source File
# Begin Source File

SOURCE=.\pacs\local_retriever.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\local_retriever.h
# End Source File
# Begin Source File

SOURCE=.\pacs\retrievable_surface.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\retrievable_surface.h
# End Source File
# Begin Source File

SOURCE=.\pacs\retriever_bank.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\retriever_bank.h
# End Source File
# Begin Source File

SOURCE=.\pacs\retriever_instance.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\retriever_instance.h
# End Source File
# Begin Source File

SOURCE=.\pacs\surface_quad.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\surface_quad.h
# End Source File
# End Group
# Begin Group "User"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\nel\pacs\u_collision_desc.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\pacs\u_global_position.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\pacs\u_global_retriever.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\pacs\u_move_container.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\pacs\u_move_primitive.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\pacs\u_primitive_block.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\pacs\u_retriever_bank.h
# End Source File
# End Group
# Begin Group "UserImp"

# PROP Default_Filter ""
# End Group
# Begin Group "Build"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pacs\build_indoor.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\build_indoor.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\pacs\Makefile.am
# End Source File
# Begin Source File

SOURCE=.\pacs\quad_grid.h
# End Source File
# Begin Source File

SOURCE=.\pacs\stdpacs.cpp
# ADD CPP /Yc"stdpacs.h"
# End Source File
# Begin Source File

SOURCE=.\pacs\stdpacs.h
# End Source File
# Begin Source File

SOURCE=.\pacs\vector_2s.cpp
# End Source File
# Begin Source File

SOURCE=.\pacs\vector_2s.h
# End Source File
# End Target
# End Project
