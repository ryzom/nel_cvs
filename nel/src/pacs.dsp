# Microsoft Developer Studio Project File - Name="pacs" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=pacs - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "pacs.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "pacs.mak" CFG="pacs - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "pacs - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "pacs - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "pacs - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "pacs___Win32_Release"
# PROP BASE Intermediate_Dir "pacs___Win32_Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "pacs___Win32_Release"
# PROP Intermediate_Dir "pacs___Win32_Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "pacs - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "pacs___Win32_Debug"
# PROP BASE Intermediate_Dir "pacs___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "__STL_DEBUG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "pacs - Win32 Release"
# Name "pacs - Win32 Debug"
# Begin Group "Pathfinding"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pacs\chain.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\pacs\chain.h
# End Source File
# Begin Source File

SOURCE=.\pacs\local_retriever.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\pacs\local_retriever.h
# End Source File
# Begin Source File

SOURCE=.\pacs\retrievable_surface.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\pacs\retrievable_surface.h
# End Source File
# Begin Source File

SOURCE=.\pacs\retriever_instance.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\pacs\retriever_instance.h
# End Source File
# Begin Source File

SOURCE=.\pacs\surface_quad.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\pacs\surface_quad.h
# End Source File
# End Group
# Begin Group "Collisions"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\pacs\collision_desc.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\pacs\collision_desc.h
# End Source File
# Begin Source File

SOURCE=.\pacs\edge_collide.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\pacs\edge_collide.h
# End Source File
# Begin Source File

SOURCE=.\pacs\move_container.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\pacs\move_container.h
# End Source File
# Begin Source File

SOURCE=.\pacs\move_primitive.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\pacs\move_primitive.h
# End Source File
# End Group
# End Target
# End Project
