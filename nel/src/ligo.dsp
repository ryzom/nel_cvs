# Microsoft Developer Studio Project File - Name="ligo" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=ligo - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "ligo.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "ligo.mak" CFG="ligo - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "ligo - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "ligo - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "ligo - Win32 DebugFast" (based on "Win32 (x86) Static Library")
!MESSAGE "ligo - Win32 ReleaseDebug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "ligo - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../obj/Release"
# PROP Intermediate_Dir "../obj/Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "_MBCS" /D "_LIB" /D "WIN32" /D "NDEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/ligo.lib"

!ELSEIF  "$(CFG)" == "ligo - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../obj/Debug"
# PROP Intermediate_Dir "../obj/Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /D "_LIB" /D "_MBCS" /D "__STL_DEBUG" /D "WIN32" /D "_DEBUG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/ligo_debug.lib"

!ELSEIF  "$(CFG)" == "ligo - Win32 DebugFast"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugFast"
# PROP BASE Intermediate_Dir "DebugFast"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../obj/DebugFast"
# PROP Intermediate_Dir "../obj/DebugFast"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /Ob1 /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "NL_DEBUG_FAST" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/ligo_debug_fast.lib"

!ELSEIF  "$(CFG)" == "ligo - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseDebug"
# PROP BASE Intermediate_Dir "ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../obj/ReleaseDebug"
# PROP Intermediate_Dir "../obj/ReleaseDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "_MBCS" /D "_LIB" /D "WIN32" /D "NDEBUG" /D "NL_RELEASE_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/ligo_rd.lib"

!ENDIF 

# Begin Target

# Name "ligo - Win32 Release"
# Name "ligo - Win32 Debug"
# Name "ligo - Win32 DebugFast"
# Name "ligo - Win32 ReleaseDebug"
# Begin Source File

SOURCE=.\ligo\ligo_config.cpp
# End Source File
# Begin Source File

SOURCE=.\ligo\ligo_config.h
# End Source File
# Begin Source File

SOURCE=.\ligo\ligo_error.cpp
# End Source File
# Begin Source File

SOURCE=.\ligo\ligo_error.h
# End Source File
# Begin Source File

SOURCE=.\ligo\ligo_material.cpp
# End Source File
# Begin Source File

SOURCE=.\ligo\ligo_material.h
# End Source File
# Begin Source File

SOURCE=.\ligo\primitive.cpp
# End Source File
# Begin Source File

SOURCE=.\ligo\transition.cpp
# End Source File
# Begin Source File

SOURCE=.\ligo\transition.h
# End Source File
# Begin Source File

SOURCE=.\ligo\zone_bank.cpp
# End Source File
# Begin Source File

SOURCE=.\ligo\zone_bank.h
# End Source File
# Begin Source File

SOURCE=.\ligo\zone_edge.cpp
# End Source File
# Begin Source File

SOURCE=.\ligo\zone_edge.h
# End Source File
# Begin Source File

SOURCE=.\ligo\zone_region.cpp
# End Source File
# Begin Source File

SOURCE=.\ligo\zone_region.h
# End Source File
# Begin Source File

SOURCE=.\ligo\zone_template.cpp
# End Source File
# Begin Source File

SOURCE=.\ligo\zone_template.h
# End Source File
# End Target
# End Project
