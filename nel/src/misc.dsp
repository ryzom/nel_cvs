# Microsoft Developer Studio Project File - Name="misc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=misc - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "misc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "misc.mak" CFG="misc - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "misc - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "misc - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "misc - Win32 ReleaseDebug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "misc - Win32 Release"

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
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/nlmisc.lib"

!ELSEIF  "$(CFG)" == "misc - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "misc___Win32_Debug"
# PROP BASE Intermediate_Dir "misc___Win32_Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "__STL_DEBUG" /FR /YX /FD /GZ /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/nlmisc_debug.lib"

!ELSEIF  "$(CFG)" == "misc - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "misc___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "misc___Win32_ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "ReleaseDebug"
# PROP Intermediate_Dir "ReleaseDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=xilink6.exe -lib
# ADD BASE LIB32 /nologo /out:"../lib/nlmisc.lib"
# ADD LIB32 /nologo /out:"../lib/nlmisc.lib"

!ENDIF 

# Begin Target

# Name "misc - Win32 Release"
# Name "misc - Win32 Debug"
# Name "misc - Win32 ReleaseDebug"
# Begin Group "stream"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\misc\class_registry.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\class_registry.h
# End Source File
# Begin Source File

SOURCE=.\misc\file.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\file.h
# End Source File
# Begin Source File

SOURCE=.\misc\stream.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\stream.h
# End Source File
# Begin Source File

SOURCE=.\misc\stream_inline.h
# End Source File
# End Group
# Begin Group "math"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\misc\matrix.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\matrix.h
# End Source File
# Begin Source File

SOURCE=.\misc\plane.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\plane.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\plane_inline.h
# End Source File
# Begin Source File

SOURCE=.\misc\quaternion.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\quaternion.h
# End Source File
# Begin Source File

SOURCE=.\misc\vector.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\vector.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\vector_inline.h
# End Source File
# Begin Source File

SOURCE=.\misc\vectord.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\vectord.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\vectord_inline.h
# End Source File
# End Group
# Begin Group "config_file"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\misc\config_file\config_file.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\config_file.h
# End Source File
# Begin Source File

SOURCE=.\misc\config_file\config_file.lex
# End Source File
# Begin Source File

SOURCE=.\misc\config_file\config_file.lex.cpp
# End Source File
# Begin Source File

SOURCE=.\misc\config_file\config_file.yacc

!IF  "$(CFG)" == "misc - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "misc - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "misc - Win32 ReleaseDebug"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\misc\config_file\config_file.yacc.cpp
# End Source File
# Begin Source File

SOURCE=.\misc\config_file\config_file.yacc.cpp.h
# End Source File
# End Group
# Begin Group "Events"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\misc\event_emitter.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\event_emitter.h
# End Source File
# Begin Source File

SOURCE=.\misc\event_listener.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\event_listener.h
# End Source File
# Begin Source File

SOURCE=.\misc\event_server.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\event_server.h
# End Source File
# Begin Source File

SOURCE=.\misc\events.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\events.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\misc\bit_set.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\bit_set.h
# End Source File
# Begin Source File

SOURCE=.\misc\class_id.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\class_id.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\common.h
# End Source File
# Begin Source File

SOURCE=.\misc\debug.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\debug.h
# End Source File
# Begin Source File

SOURCE=.\misc\displayer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\displayer.h
# End Source File
# Begin Source File

SOURCE=.\misc\i18n.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\i18n.h
# End Source File
# Begin Source File

SOURCE=.\misc\log.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\log.h
# End Source File
# Begin Source File

SOURCE=.\misc\rgba.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\rgba.h
# End Source File
# Begin Source File

SOURCE=.\misc\smart_ptr.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\smart_ptr.h
# End Source File
# Begin Source File

SOURCE=.\misc\time_nl.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\time_nl.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\types_nl.h
# End Source File
# End Target
# End Project
