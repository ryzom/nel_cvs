# Microsoft Developer Studio Project File - Name="georges_lib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=georges_lib - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "georges_lib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "georges_lib.mak" CFG="georges_lib - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "georges_lib - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "georges_lib - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "georges_lib - Win32 DebugFast" (based on "Win32 (x86) Static Library")
!MESSAGE "georges_lib - Win32 ReleaseDebug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "georges_lib - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "georges_lib - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ  /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ  /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "georges_lib - Win32 DebugFast"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugFast"
# PROP BASE Intermediate_Dir "DebugFast"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugFast"
# PROP Intermediate_Dir "DebugFast"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ  /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "NL_DEBUG_FAST" /FD /GZ  /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ELSEIF  "$(CFG)" == "georges_lib - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseDebug"
# PROP BASE Intermediate_Dir "ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
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

# Name "georges_lib - Win32 Release"
# Name "georges_lib - Win32 Debug"
# Name "georges_lib - Win32 DebugFast"
# Name "georges_lib - Win32 ReleaseDebug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\Form.cpp
# End Source File
# Begin Source File

SOURCE=.\FormBodyElt.cpp
# End Source File
# Begin Source File

SOURCE=.\FormBodyEltAtom.cpp
# End Source File
# Begin Source File

SOURCE=.\FormBodyEltList.cpp
# End Source File
# Begin Source File

SOURCE=.\FormBodyEltStruct.cpp
# End Source File
# Begin Source File

SOURCE=.\FormFile.cpp
# End Source File
# Begin Source File

SOURCE=.\FormHead.cpp
# End Source File
# Begin Source File

SOURCE=.\FormLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\Item.cpp
# End Source File
# Begin Source File

SOURCE=.\ItemElt.cpp
# End Source File
# Begin Source File

SOURCE=.\ItemEltAtom.cpp
# End Source File
# Begin Source File

SOURCE=.\ItemEltList.cpp
# End Source File
# Begin Source File

SOURCE=.\ItemEltStruct.cpp
# End Source File
# Begin Source File

SOURCE=.\ItemLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\Loader.cpp
# End Source File
# Begin Source File

SOURCE=.\MoldElt.cpp
# End Source File
# Begin Source File

SOURCE=.\MoldEltDefine.cpp
# End Source File
# Begin Source File

SOURCE=.\MoldEltType.cpp
# End Source File
# Begin Source File

SOURCE=.\MoldLoader.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\StringEx.cpp
# End Source File
# Begin Source File

SOURCE=.\TypeUnit.cpp
# End Source File
# Begin Source File

SOURCE=.\TypeUnitDouble.cpp
# End Source File
# Begin Source File

SOURCE=.\TypeUnitIntSigned.cpp
# End Source File
# Begin Source File

SOURCE=.\TypeUnitIntUnsigned.cpp
# End Source File
# Begin Source File

SOURCE=.\TypeUnitString.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\common.h
# End Source File
# Begin Source File

SOURCE=.\Form.h
# End Source File
# Begin Source File

SOURCE=.\FormBodyElt.h
# End Source File
# Begin Source File

SOURCE=.\FormBodyEltAtom.h
# End Source File
# Begin Source File

SOURCE=.\FormBodyEltList.h
# End Source File
# Begin Source File

SOURCE=.\FormBodyEltStruct.h
# End Source File
# Begin Source File

SOURCE=.\FormFile.h
# End Source File
# Begin Source File

SOURCE=.\FormHead.h
# End Source File
# Begin Source File

SOURCE=.\FormLoader.h
# End Source File
# Begin Source File

SOURCE=.\Item.h
# End Source File
# Begin Source File

SOURCE=.\ItemElt.h
# End Source File
# Begin Source File

SOURCE=.\ItemEltAtom.h
# End Source File
# Begin Source File

SOURCE=.\ItemEltList.h
# End Source File
# Begin Source File

SOURCE=.\ItemEltStruct.h
# End Source File
# Begin Source File

SOURCE=.\ItemLoader.h
# End Source File
# Begin Source File

SOURCE=.\Loader.h
# End Source File
# Begin Source File

SOURCE=.\MoldElt.h
# End Source File
# Begin Source File

SOURCE=.\MoldEltDefine.h
# End Source File
# Begin Source File

SOURCE=.\MoldEltType.h
# End Source File
# Begin Source File

SOURCE=.\MoldLoader.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\StringEx.h
# End Source File
# Begin Source File

SOURCE=.\TypeUnit.h
# End Source File
# Begin Source File

SOURCE=.\TypeUnitDouble.h
# End Source File
# Begin Source File

SOURCE=.\TypeUnitIntSigned.h
# End Source File
# Begin Source File

SOURCE=.\TypeUnitIntUnsigned.h
# End Source File
# Begin Source File

SOURCE=.\TypeUnitString.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\Readme.txt
# End Source File
# End Target
# End Project
