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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "__STL_DEBUG" /FR /YX /FD /GZ /c
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
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD BASE LIB32 /nologo /out:"../lib/nl3d.lib"
# ADD LIB32 /nologo /out:"../lib/nl3d.lib"

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

SOURCE=.\3d\tessellation.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tessellation.h
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

SOURCE=.\3d\material.cpp
# End Source File
# Begin Source File

SOURCE=.\3d\primitive_block.cpp
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
# End Group
# Begin Source File

SOURCE=.\3d\bitmap.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\bitmap.h
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

SOURCE=.\3d\scene.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\scene.h
# End Source File
# Begin Source File

SOURCE=.\3d\scene_ut.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\scene_ut.h
# End Source File
# Begin Source File

SOURCE=.\3d\tile_bank.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\3d\tile_bank.h
# End Source File
# End Target
# End Project
