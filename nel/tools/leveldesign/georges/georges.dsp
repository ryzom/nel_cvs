# Microsoft Developer Studio Project File - Name="georges" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=georges - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "georges.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "georges.mak" CFG="georges - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "georges - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "georges - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "georges - Win32 DebugFast" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "georges - Win32 ReleaseDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "georges - Win32 Release"

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

!ELSEIF  "$(CFG)" == "georges - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../georges_exe/georges.dll" /pdbtype:sept

!ELSEIF  "$(CFG)" == "georges - Win32 DebugFast"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "georges___Win32_DebugFast"
# PROP BASE Intermediate_Dir "georges___Win32_DebugFast"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "DebugFast"
# PROP Intermediate_Dir "DebugFast"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../georges_exe/georges.dll" /pdbtype:sept
# ADD LINK32 libxml2.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../master/georges_debug_fast.dll" /pdbtype:sept
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\master\georges_debug_fast.dll ..\georges_exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "georges - Win32 ReleaseDebug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "georges___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "georges___Win32_ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "NL_RELEASE_DEBUG" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x409 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 libxml2.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../master/georges_release_debug.dll"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy ..\master\georges_release_debug.dll ..\georges_exe
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "georges - Win32 Release"
# Name "georges - Win32 Debug"
# Name "georges - Win32 DebugFast"
# Name "georges - Win32 ReleaseDebug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\ComboInListView.cpp
# End Source File
# Begin Source File

SOURCE=.\DfnDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\DfnFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\DfnGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\DfnView.cpp
# End Source File
# Begin Source File

SOURCE=.\georges.cpp
# End Source File
# Begin Source File

SOURCE=.\georges_implementation.cpp
# End Source File
# Begin Source File

SOURCE=.\georgesDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\georgesFrame.cpp
# End Source File
# Begin Source File

SOURCE=.\georgesGrid.cpp
# End Source File
# Begin Source File

SOURCE=.\georgesView.cpp
# End Source File
# Begin Source File

SOURCE=.\listeditctrl.cpp
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\newitem.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\supergridctrl.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\ComboInListView.h
# End Source File
# Begin Source File

SOURCE=.\DfnDoc.h
# End Source File
# Begin Source File

SOURCE=.\DfnFrame.h
# End Source File
# Begin Source File

SOURCE=.\DfnGrid.h
# End Source File
# Begin Source File

SOURCE=.\DfnView.h
# End Source File
# Begin Source File

SOURCE=.\edit_stack.h
# End Source File
# Begin Source File

SOURCE=.\georges.h
# End Source File
# Begin Source File

SOURCE=.\georges_interface.h
# End Source File
# Begin Source File

SOURCE=.\georgesDoc.h
# End Source File
# Begin Source File

SOURCE=.\georgesFrame.h
# End Source File
# Begin Source File

SOURCE=.\georgesGrid.h
# End Source File
# Begin Source File

SOURCE=.\georgesView.h
# End Source File
# Begin Source File

SOURCE=.\listeditctrl.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\newitem.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\supergridctrl.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\folders.bmp
# End Source File
# Begin Source File

SOURCE=.\res\georges.ico
# End Source File
# Begin Source File

SOURCE=.\georges.rc
# End Source File
# Begin Source File

SOURCE=.\res\georges.rc2
# End Source File
# Begin Source File

SOURCE=.\res\georgesDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\idr_type.ico
# End Source File
# Begin Source File

SOURCE=.\res\subitems.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\georges.def

!IF  "$(CFG)" == "georges - Win32 Release"

!ELSEIF  "$(CFG)" == "georges - Win32 Debug"

!ELSEIF  "$(CFG)" == "georges - Win32 DebugFast"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "georges - Win32 ReleaseDebug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\georges_debug_fast.def

!IF  "$(CFG)" == "georges - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "georges - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "georges - Win32 DebugFast"

# PROP Intermediate_Dir "DebugFast"

!ELSEIF  "$(CFG)" == "georges - Win32 ReleaseDebug"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\georges_release_debug.def

!IF  "$(CFG)" == "georges - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "georges - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "georges - Win32 DebugFast"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "georges - Win32 ReleaseDebug"

!ENDIF 

# End Source File
# End Target
# End Project
