# Microsoft Developer Studio Project File - Name="object_viewer" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=object_viewer - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "object_viewer.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "object_viewer.mak" CFG="object_viewer - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "object_viewer - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "object_viewer - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "object_viewer - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"std_afx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /machine:I386
# ADD LINK32 mfcs42.lib msvcrt.lib libc.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../../../lib/object_viewer.dll"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "object_viewer - Win32 Debug"

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /D "__STL_DEBUG" /Yu"std_afx.h" /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 mfcs42d.lib msvcrtd.lib libc.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../../lib/object_viewer_debug.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "object_viewer - Win32 Release"
# Name "object_viewer - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\animation_dlg.cpp

!IF  "$(CFG)" == "object_viewer - Win32 Release"

# ADD CPP /Yu"std_afx.h"

!ELSEIF  "$(CFG)" == "object_viewer - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\animation_set_dlg.cpp

!IF  "$(CFG)" == "object_viewer - Win32 Release"

# ADD CPP /Yu"std_afx.h"

!ELSEIF  "$(CFG)" == "object_viewer - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\blend_wnd.cpp

!IF  "$(CFG)" == "object_viewer - Win32 Release"

# ADD CPP /Yu"std_afx.h"

!ELSEIF  "$(CFG)" == "object_viewer - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\main_dlg.cpp

!IF  "$(CFG)" == "object_viewer - Win32 Release"

# ADD CPP /Yu"std_afx.h"

!ELSEIF  "$(CFG)" == "object_viewer - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\object_viewer.cpp
# ADD CPP /Yu"std_afx.h"
# End Source File
# Begin Source File

SOURCE=.\object_viewer.rc
# End Source File
# Begin Source File

SOURCE=.\scene_dlg.cpp

!IF  "$(CFG)" == "object_viewer - Win32 Release"

# ADD CPP /Yu"std_afx.h"

!ELSEIF  "$(CFG)" == "object_viewer - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\select_string.cpp

!IF  "$(CFG)" == "object_viewer - Win32 Release"

# ADD CPP /Yu"std_afx.h"

!ELSEIF  "$(CFG)" == "object_viewer - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\slot_dlg.cpp

!IF  "$(CFG)" == "object_viewer - Win32 Release"

# ADD CPP /Yu"std_afx.h"

!ELSEIF  "$(CFG)" == "object_viewer - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\std_afx.cpp
# ADD CPP /Yc"std_afx.h"
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\animation_dlg.h
# End Source File
# Begin Source File

SOURCE=.\animation_set_dlg.h
# End Source File
# Begin Source File

SOURCE=.\blend_wnd.h
# End Source File
# Begin Source File

SOURCE=.\main_dlg.h
# End Source File
# Begin Source File

SOURCE=.\object_viewer.h
# End Source File
# Begin Source File

SOURCE=.\object_viewer_interface.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\scene_dlg.h
# End Source File
# Begin Source File

SOURCE=.\select_string.h
# End Source File
# Begin Source File

SOURCE=.\slot_dlg.h
# End Source File
# Begin Source File

SOURCE=.\std_afx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\object_viewer.rc2
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
