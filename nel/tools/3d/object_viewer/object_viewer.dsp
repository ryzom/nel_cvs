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
!MESSAGE "object_viewer - Win32 ReleaseDebug" (based on "Win32 (x86) Dynamic-Link Library")
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
# ADD LINK32 mfcs42d.lib msvcrtd.lib libc.lib /nologo /subsystem:windows /dll /incremental:no /debug /machine:I386 /out:"../../../lib/object_viewer_debug.dll" /pdbtype:sept
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "object_viewer - Win32 ReleaseDebug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "object_viewer___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "object_viewer___Win32_ReleaseDebug"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"std_afx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_WINDLL" /D "_AFXDLL" /D "_MBCS" /D "_USRDLL" /Yu"std_afx.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x40c /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 mfcs42.lib msvcrt.lib libc.lib /nologo /subsystem:windows /dll /machine:I386 /out:"../../../lib/object_viewer.dll"
# ADD LINK32 mfcs42.lib msvcrt.lib libc.lib /nologo /subsystem:windows /dll /debug /machine:I386 /out:"../../../lib/object_viewer_rd.dll"

!ENDIF 

# Begin Target

# Name "object_viewer - Win32 Release"
# Name "object_viewer - Win32 Debug"
# Name "object_viewer - Win32 ReleaseDebug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\animation_dlg.cpp
# ADD CPP /Yu"std_afx.h"
# End Source File
# Begin Source File

SOURCE=.\animation_set_dlg.cpp
# ADD CPP /Yu"std_afx.h"
# End Source File
# Begin Source File

SOURCE=.\blend_wnd.cpp
# ADD CPP /Yu"std_afx.h"
# End Source File
# Begin Source File

SOURCE=.\main_dlg.cpp
# ADD CPP /Yu"std_afx.h"
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
# ADD CPP /Yu"std_afx.h"
# End Source File
# Begin Source File

SOURCE=.\select_string.cpp
# ADD CPP /Yu"std_afx.h"
# End Source File
# Begin Source File

SOURCE=.\slot_dlg.cpp
# ADD CPP /Yu"std_afx.h"
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

SOURCE=.\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\bitmap2.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00002.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00003.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00004.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00005.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00006.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00007.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00008.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00009.bmp
# End Source File
# Begin Source File

SOURCE=.\bmp00010.bmp
# End Source File
# Begin Source File

SOURCE=.\res\object_viewer.rc2
# End Source File
# Begin Source File

SOURCE=.\particle.bmp
# End Source File
# End Group
# Begin Group "particles editor"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\attrib_dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\attrib_dlg.h
# End Source File
# Begin Source File

SOURCE=.\attrib_list_box.cpp
# End Source File
# Begin Source File

SOURCE=.\attrib_list_box.h
# End Source File
# Begin Source File

SOURCE=.\color_button.cpp
# End Source File
# Begin Source File

SOURCE=.\color_button.h
# End Source File
# Begin Source File

SOURCE=.\color_edit.cpp
# End Source File
# Begin Source File

SOURCE=.\color_edit.h
# End Source File
# Begin Source File

SOURCE=.\edit_attrib_dlg.h
# End Source File
# Begin Source File

SOURCE=.\editable_range.cpp
# End Source File
# Begin Source File

SOURCE=.\editable_range.h
# End Source File
# Begin Source File

SOURCE=.\located_bindable_dialog.cpp
# End Source File
# Begin Source File

SOURCE=.\located_bindable_dialog.h
# End Source File
# Begin Source File

SOURCE=.\located_properties.cpp
# End Source File
# Begin Source File

SOURCE=.\located_properties.h
# End Source File
# Begin Source File

SOURCE=.\particle_dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\particle_dlg.h
# End Source File
# Begin Source File

SOURCE=.\particle_dsw.h
# End Source File
# Begin Source File

SOURCE=.\particle_tree_ctrl.cpp
# End Source File
# Begin Source File

SOURCE=.\particle_tree_ctrl.h
# End Source File
# Begin Source File

SOURCE=.\ps_wrapper.h
# End Source File
# Begin Source File

SOURCE=.\range_manager.cpp
# End Source File
# Begin Source File

SOURCE=.\range_manager.h
# End Source File
# Begin Source File

SOURCE=.\range_selector.cpp
# End Source File
# Begin Source File

SOURCE=.\range_selector.h
# End Source File
# Begin Source File

SOURCE=.\start_stop_particle_system.cpp
# End Source File
# Begin Source File

SOURCE=.\start_stop_particle_system.h
# End Source File
# Begin Source File

SOURCE=.\value_blender_dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\value_blender_dlg.h
# End Source File
# Begin Source File

SOURCE=.\value_gradient_dlg.cpp
# End Source File
# Begin Source File

SOURCE=.\value_gradient_dlg.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
