# Microsoft Developer Studio Project File - Name="lib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=lib - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "lib.mak" CFG="lib - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "lib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "lib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "lib - Win32 DebugFast" (based on "Win32 (x86) Static Library")
!MESSAGE "lib - Win32 ReleaseDebug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "lib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "lib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "_MBCS" /D "_LIB" /D "WIN32" /D "_DEBUG" /D "__STL_DEBUG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "lib - Win32 DebugFast"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "lib___Win32_DebugFast"
# PROP BASE Intermediate_Dir "lib___Win32_DebugFast"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugFast"
# PROP Intermediate_Dir "DebugFast"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "_MBCS" /D "_LIB" /D "WIN32" /D "_DEBUG" /D "__STL_DEBUG" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /D "_MBCS" /D "_LIB" /D "WIN32" /D "_DEBUG" /D "NL_DEBUG_FAST" /YX /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "lib - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "lib___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "lib___Win32_ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NL_RELEASE_DEBUG" /YX /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "lib - Win32 Release"
# Name "lib - Win32 Debug"
# Name "lib - Win32 DebugFast"
# Name "lib - Win32 ReleaseDebug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ligo_config.cpp
# End Source File
# Begin Source File

SOURCE=.\ligo_error.cpp
# End Source File
# Begin Source File

SOURCE=.\material.cpp
# End Source File
# Begin Source File

SOURCE=.\transition.cpp
# End Source File
# Begin Source File

SOURCE=.\zone_bank.cpp
# End Source File
# Begin Source File

SOURCE=.\zone_edge.cpp
# End Source File
# Begin Source File

SOURCE=.\zone_template.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ligo_config.h
# End Source File
# Begin Source File

SOURCE=.\ligo_error.h
# End Source File
# Begin Source File

SOURCE=.\material.h
# End Source File
# Begin Source File

SOURCE=.\transition.h
# End Source File
# Begin Source File

SOURCE=.\zone_bank.h
# End Source File
# Begin Source File

SOURCE=.\zone_edge.h
# End Source File
# Begin Source File

SOURCE=.\zone_template.h
# End Source File
# End Group
# Begin Group "Configuration Files"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\ligoscape.cfg
# End Source File
# End Group
# Begin Group "Logical"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\export.cpp
# End Source File
# Begin Source File

SOURCE=.\export.h
# End Source File
# Begin Source File

SOURCE=.\primitive.cpp
# End Source File
# Begin Source File

SOURCE=.\primitive.h
# End Source File
# Begin Source File

SOURCE=.\zone_region.cpp
# End Source File
# Begin Source File

SOURCE=.\zone_region.h
# End Source File
# End Group
# End Target
# End Project
