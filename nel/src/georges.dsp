# Microsoft Developer Studio Project File - Name="georges" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

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
!MESSAGE "georges - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "georges - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "georges - Win32 DebugFast" (based on "Win32 (x86) Static Library")
!MESSAGE "georges - Win32 ReleaseDebug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "georges - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdgeorges.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/georges.lib"

!ELSEIF  "$(CFG)" == "georges - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "_MBCS" /D "_LIB" /D "__STL_DEBUG" /D "WIN32" /D "_DEBUG" /Yu"stdgeorges.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/georges_debug.lib"

!ELSEIF  "$(CFG)" == "georges - Win32 DebugFast"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /Zi /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "NL_DEBUG_FAST" /Yu"stdgeorges.h" /FD /GZ /c
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/georges_debug_fast.lib"

!ELSEIF  "$(CFG)" == "georges - Win32 ReleaseDebug"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NL_RELEASE_DEBUG" /Yu"stdgeorges.h" /FD /c
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/georges_rd.lib"

!ENDIF 

# Begin Target

# Name "georges - Win32 Release"
# Name "georges - Win32 Debug"
# Name "georges - Win32 DebugFast"
# Name "georges - Win32 ReleaseDebug"
# Begin Source File

SOURCE=..\include\nel\georges\common.h
# End Source File
# Begin Source File

SOURCE=.\georges\Form.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\Form.h
# End Source File
# Begin Source File

SOURCE=.\georges\form_body_elt.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\form_body_elt.h
# End Source File
# Begin Source File

SOURCE=.\georges\form_body_elt_atom.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\form_body_elt_atom.h
# End Source File
# Begin Source File

SOURCE=.\georges\form_body_elt_list.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\form_body_elt_list.h
# End Source File
# Begin Source File

SOURCE=.\georges\form_body_elt_struct.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\form_body_elt_struct.h
# End Source File
# Begin Source File

SOURCE=.\georges\form_file.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\form_file.h
# End Source File
# Begin Source File

SOURCE=.\georges\form_head.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\form_head.h
# End Source File
# Begin Source File

SOURCE=.\georges\form_loader.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\form_loader.h
# End Source File
# Begin Source File

SOURCE=.\georges\georges_loader.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\georges_loader.h
# End Source File
# Begin Source File

SOURCE=.\georges\Item.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\Item.h
# End Source File
# Begin Source File

SOURCE=.\georges\item_elt.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\item_elt.h
# End Source File
# Begin Source File

SOURCE=.\georges\item_elt_atom.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\item_elt_atom.h
# End Source File
# Begin Source File

SOURCE=.\georges\item_elt_list.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\item_elt_list.h
# End Source File
# Begin Source File

SOURCE=.\georges\item_elt_struct.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\item_elt_struct.h
# End Source File
# Begin Source File

SOURCE=.\georges\item_loader.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\item_loader.h
# End Source File
# Begin Source File

SOURCE=.\georges\mold_elt.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\mold_elt.h
# End Source File
# Begin Source File

SOURCE=.\georges\mold_elt_define.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\mold_elt_define.h
# End Source File
# Begin Source File

SOURCE=.\georges\mold_elt_define_list.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\mold_elt_define_list.h
# End Source File
# Begin Source File

SOURCE=.\georges\mold_elt_type.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\mold_elt_type.h
# End Source File
# Begin Source File

SOURCE=.\georges\mold_elt_type_list.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\mold_elt_type_list.h
# End Source File
# Begin Source File

SOURCE=.\georges\mold_loader.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\mold_loader.h
# End Source File
# Begin Source File

SOURCE=.\georges\stdgeorges.cpp
# ADD CPP /Yc"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=.\georges\stdgeorges.h
# End Source File
# Begin Source File

SOURCE=.\georges\string_ex.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\string_ex.h
# End Source File
# Begin Source File

SOURCE=.\georges\type_unit.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\type_unit.h
# End Source File
# Begin Source File

SOURCE=.\georges\type_unit_double.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\type_unit_double.h
# End Source File
# Begin Source File

SOURCE=.\georges\type_unit_file_name.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\type_unit_file_name.h
# End Source File
# Begin Source File

SOURCE=.\georges\type_unit_int_signed.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\type_unit_int_signed.h
# End Source File
# Begin Source File

SOURCE=.\georges\type_unit_int_unsigned.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\type_unit_int_unsigned.h
# End Source File
# Begin Source File

SOURCE=.\georges\type_unit_string.cpp
# ADD CPP /Yu"stdgeorges.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\georges\type_unit_string.h
# End Source File
# End Target
# End Project
