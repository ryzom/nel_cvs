# Microsoft Developer Studio Project File - Name="driver_dsound" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=driver_dsound - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "driver_dsound.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "driver_dsound.mak" CFG="driver_dsound - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "driver_dsound - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "driver_dsound - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "driver_dsound - Win32 ReleaseDebug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "driver_dsound - Win32 DebugFast" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "driver_dsound - Win32 Release"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../obj/Release/driver_dsound"
# PROP Intermediate_Dir "../../../../obj/Release/driver_dsound"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "NDEBUG" /D "NLSOUND_PROFILE" /D "_WINDOWS" /D "_USRDLL" /D "DRIVER_DSOUND_EXPORTS" /D "WIN32" /D "_MBCS" /D "LIBXML_STATIC" /FD /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 nlsound_lowlevel_r.lib nlmisc_r.lib eax.lib eaxguid.lib user32.lib gdi32.lib advapi32.lib dsound.lib /nologo /dll /pdb:none /machine:I386 /out:"../../../../lib/nel_drv_dsound_win_r.dll"

!ELSEIF  "$(CFG)" == "driver_dsound - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../obj/Debug/driver_dsound"
# PROP Intermediate_Dir "../../../../obj/Debug/driver_dsound"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /D "_DEBUG" /D "__STL_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "DRIVER_DSOUND_EXPORTS" /D "WIN32" /D "_MBCS" /D "LIBXML_STATIC" /FD /GZ /c
# SUBTRACT CPP /Gf /Gy /Fr /YX /Yc /Yu
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 nlsound_lowlevel_d.lib kernel32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib nlmisc_d.lib eax.lib eaxguid.lib user32.lib gdi32.lib advapi32.lib dsound.lib /nologo /dll /incremental:no /debug /machine:I386 /out:"../../../../lib/nel_drv_dsound_win_d.dll"

!ELSEIF  "$(CFG)" == "driver_dsound - Win32 ReleaseDebug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../../../../obj/ReleaseDebug/driver_dsound"
# PROP Intermediate_Dir "../../../../obj/ReleaseDebug/driver_dsound"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "NDEBUG" /D "NL_RELEASE_DEBUG" /D "_WINDOWS" /D "_USRDLL" /D "DRIVER_DSOUND_EXPORTS" /D "WIN32" /D "_MBCS" /D "LIBXML_STATIC" /FD /c
# SUBTRACT CPP /Fr /YX /Yc /Yu
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 nlsound_lowlevel_rd.lib kernel32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib nlmisc_rd.lib eax.lib eaxguid.lib user32.lib gdi32.lib advapi32.lib dsound.lib /nologo /dll /debug /machine:I386 /out:"../../../../lib/nel_drv_dsound_win_rd.dll"
# SUBTRACT LINK32 /incremental:yes

!ELSEIF  "$(CFG)" == "driver_dsound - Win32 DebugFast"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../../../../obj/DebugFast/driver_dsound"
# PROP Intermediate_Dir "../../../../obj/DebugFast/driver_dsound"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /Ob1 /D "_DEBUG" /D "NL_DEBUG_FAST" /D "_WINDOWS" /D "_USRDLL" /D "DRIVER_DSOUND_EXPORTS" /D "WIN32" /D "_MBCS" /D "LIBXML_STATIC" /FD /GZ /c
# SUBTRACT CPP /Gf /Gy /Fr /YX /Yc /Yu
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40c
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /machine:IX86
# ADD LINK32 nlsound_lowlevel_df.lib kernel32.lib winspool.lib comdlg32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib nlmisc_df.lib eax.lib eaxguid.lib user32.lib gdi32.lib advapi32.lib dsound.lib /nologo /dll /incremental:no /debug /machine:I386 /out:"../../../../lib/nel_drv_dsound_win_df.dll"

!ENDIF 

# Begin Target

# Name "driver_dsound - Win32 Release"
# Name "driver_dsound - Win32 Debug"
# Name "driver_dsound - Win32 ReleaseDebug"
# Name "driver_dsound - Win32 DebugFast"
# Begin Source File

SOURCE=..\dsound\buffer_dsound.cpp
# End Source File
# Begin Source File

SOURCE=..\dsound\buffer_dsound.h
# End Source File
# Begin Source File

SOURCE=..\dsound\driver_dsound.def
# End Source File
# Begin Source File

SOURCE=..\dsound\listener_dsound.cpp
# End Source File
# Begin Source File

SOURCE=..\dsound\listener_dsound.h
# End Source File
# Begin Source File

SOURCE=..\dsound\sound_driver_dsound.cpp
# End Source File
# Begin Source File

SOURCE=..\dsound\sound_driver_dsound.h
# End Source File
# Begin Source File

SOURCE=..\dsound\source_dsound.cpp
# End Source File
# Begin Source File

SOURCE=..\dsound\source_dsound.h
# End Source File
# Begin Source File

SOURCE=..\dsound\stddsound.cpp

!IF  "$(CFG)" == "driver_dsound - Win32 Release"

!ELSEIF  "$(CFG)" == "driver_dsound - Win32 Debug"

!ELSEIF  "$(CFG)" == "driver_dsound - Win32 ReleaseDebug"

!ELSEIF  "$(CFG)" == "driver_dsound - Win32 DebugFast"

# ADD CPP /Yc"stddsound.h"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\dsound\stddsound.h
# End Source File
# End Target
# End Project
