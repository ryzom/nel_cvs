# Microsoft Developer Studio Project File - Name="WorldEditor" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=WorldEditor - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "WorldEditor.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "WorldEditor.mak" CFG="WorldEditor - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "WorldEditor - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "WorldEditor - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "WorldEditor - Win32 DebugFast" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "WorldEditor - Win32 ReleaseDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "WorldEditor - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 /nologo /subsystem:windows /dll /machine:I386

!ELSEIF  "$(CFG)" == "WorldEditor - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "__STL_DEBUG" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 freetype.lib libxml2.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../../../lib/WorldEditor_debug.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "WorldEditor - Win32 DebugFast"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "DebugFast"
# PROP BASE Intermediate_Dir "DebugFast"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugFast"
# PROP Intermediate_Dir "DebugFast"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /Ob1 /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "NL_DEBUG_FAST" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 freetype.lib libxml2.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../../../lib/WorldEditor_debug_fast.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "WorldEditor - Win32 ReleaseDebug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "ReleaseDebug"
# PROP BASE Intermediate_Dir "ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "NL_RELEASE_DEBUG" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 freetype.lib libxml2.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../../../lib/WorldEditor_release_debug.dll"

!ENDIF 

# Begin Target

# Name "WorldEditor - Win32 Release"
# Name "WorldEditor - Win32 Debug"
# Name "WorldEditor - Win32 DebugFast"
# Name "WorldEditor - Win32 ReleaseDebug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\builderLogic.cpp
# End Source File
# Begin Source File

SOURCE=.\builderZone.cpp
# End Source File
# Begin Source File

SOURCE=.\builderZoneRegion.cpp
# End Source File
# Begin Source File

SOURCE=.\color_button.cpp
# End Source File
# Begin Source File

SOURCE=.\Display.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportCBDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ExportDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\GenerateDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\mainfrmView.cpp
# End Source File
# Begin Source File

SOURCE=.\MoveDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\toolsLogic.cpp
# End Source File
# Begin Source File

SOURCE=.\toolsZone.cpp
# End Source File
# Begin Source File

SOURCE=.\TypeDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TypeManagerDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\TypeSelDlg.cpp
# End Source File
# Begin Source File

SOURCE=R:\code\nel\tools\3d\ligo\WorldEditor\WorldEditor.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\builderLogic.h
# End Source File
# Begin Source File

SOURCE=.\BuilderZone.h
# End Source File
# Begin Source File

SOURCE=.\builderZoneRegion.h
# End Source File
# Begin Source File

SOURCE=.\color_button.h
# End Source File
# Begin Source File

SOURCE=.\Display.h
# End Source File
# Begin Source File

SOURCE=.\edit_stack.h
# End Source File
# Begin Source File

SOURCE=.\ExportCBDlg.h
# End Source File
# Begin Source File

SOURCE=.\ExportDlg.h
# End Source File
# Begin Source File

SOURCE=.\GenerateDlg.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MoveDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\toolsLogic.h
# End Source File
# Begin Source File

SOURCE=.\toolsZone.h
# End Source File
# Begin Source File

SOURCE=.\TypeDlg.h
# End Source File
# Begin Source File

SOURCE=.\TypeManagerDlg.h
# End Source File
# Begin Source File

SOURCE=.\TypeSelDlg.h
# End Source File
# Begin Source File

SOURCE=R:\code\nel\tools\3d\ligo\WorldEditor\WorldEditor.h
# End Source File
# Begin Source File

SOURCE=R:\code\nel\tools\3d\ligo\WorldEditor\WorldEditor_interface.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\icon1.ico
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\WorldEditor.ico
# End Source File
# Begin Source File

SOURCE=.\WorldEditor.rc
# End Source File
# Begin Source File

SOURCE=.\WorldEditor_debug.def

!IF  "$(CFG)" == "WorldEditor - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WorldEditor - Win32 Debug"

!ELSEIF  "$(CFG)" == "WorldEditor - Win32 DebugFast"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WorldEditor - Win32 ReleaseDebug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WorldEditor_debug_fast.def

!IF  "$(CFG)" == "WorldEditor - Win32 Release"

!ELSEIF  "$(CFG)" == "WorldEditor - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WorldEditor - Win32 DebugFast"

!ELSEIF  "$(CFG)" == "WorldEditor - Win32 ReleaseDebug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\WorldEditor_release_debug.def

!IF  "$(CFG)" == "WorldEditor - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WorldEditor - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WorldEditor - Win32 DebugFast"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "WorldEditor - Win32 ReleaseDebug"

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
