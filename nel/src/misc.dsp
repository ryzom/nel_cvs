# Microsoft Developer Studio Project File - Name="misc" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=misc - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "misc.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "misc.mak" CFG="misc - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "misc - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "misc - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "misc - Win32 ReleaseDebug" (based on "Win32 (x86) Static Library")
!MESSAGE "misc - Win32 DebugFast" (based on "Win32 (x86) Static Library")
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
# PROP Output_Dir "../obj/Release"
# PROP Intermediate_Dir "../obj/Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "_MBCS" /D "_LIB" /D "WIN32" /D "NDEBUG" /Yu"stdmisc.h" /FD /c
# SUBTRACT CPP /Fr
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
# PROP Output_Dir "../obj/Debug"
# PROP Intermediate_Dir "../obj/Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /Zi /Od /Gf /D "_LIB" /D "__STL_DEBUG" /D "_MBCS" /D "WIN32" /D "_DEBUG" /Yu"stdmisc.h" /FD /GZ /c
# SUBTRACT CPP /Gy /Fr
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
# PROP Output_Dir "../obj/ReleaseDebug"
# PROP Intermediate_Dir "../obj/ReleaseDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "_MBCS" /D "_LIB" /D "WIN32" /D "NDEBUG" /D "NL_RELEASE_DEBUG" /Yu"stdmisc.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"../lib/nlmisc.lib"
# ADD LIB32 /nologo /out:"../lib/nlmisc_rd.lib"

!ELSEIF  "$(CFG)" == "misc - Win32 DebugFast"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "misc___Win32_DebugFast"
# PROP BASE Intermediate_Dir "misc___Win32_DebugFast"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../obj/DebugFast"
# PROP Intermediate_Dir "../obj/DebugFast"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /Zi /Od /Gf /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "__STL_DEBUG" /Fr /FD /GZ /c
# SUBTRACT BASE CPP /Gy
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /Zi /Od /Ob1 /Gf /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "NL_DEBUG_FAST" /Yu"stdmisc.h" /FD /GZ /c
# SUBTRACT CPP /Gy /Fr
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"../lib/nlmisc_debug.lib"
# ADD LIB32 /nologo /out:"../lib/nlmisc_debug_fast.lib"

!ENDIF 

# Begin Target

# Name "misc - Win32 Release"
# Name "misc - Win32 Debug"
# Name "misc - Win32 ReleaseDebug"
# Name "misc - Win32 DebugFast"
# Begin Group "Stream"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\misc\bit_mem_stream.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\bit_mem_stream.h
# End Source File
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

SOURCE=.\misc\i_xml.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\i_xml.h
# End Source File
# Begin Source File

SOURCE=.\misc\mem_stream.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\mem_stream.h
# End Source File
# Begin Source File

SOURCE=.\misc\o_xml.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\o_xml.h
# End Source File
# Begin Source File

SOURCE=.\misc\stream.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\stream.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\stream_inline.h
# End Source File
# End Group
# Begin Group "Math"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\misc\aabbox.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\aabbox.h
# End Source File
# Begin Source File

SOURCE=.\misc\bsphere.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\bsphere.h
# End Source File
# Begin Source File

SOURCE=.\misc\geom_ext.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\geom_ext.h
# End Source File
# Begin Source File

SOURCE=.\misc\line.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\line.h
# End Source File
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

SOURCE=.\misc\polygon.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\polygon.h
# End Source File
# Begin Source File

SOURCE=.\misc\quat.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\quat.h
# End Source File
# Begin Source File

SOURCE=.\misc\rect.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\rect.h
# End Source File
# Begin Source File

SOURCE=.\misc\triangle.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\triangle.h
# End Source File
# Begin Source File

SOURCE=.\misc\uv.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\uv.h
# End Source File
# Begin Source File

SOURCE=.\misc\vector.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\vector.h
# End Source File
# Begin Source File

SOURCE=.\misc\vector_2d.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\vector_2d.h
# End Source File
# Begin Source File

SOURCE=.\misc\vector_2f.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\vector_2f.h
# End Source File
# Begin Source File

SOURCE=.\misc\vector_h.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\vector_h.h
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
# Begin Group "ConfigFile"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\misc\config_file\config_file.cpp
# ADD CPP /Yu"../stdmisc.h"
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\config_file.h
# End Source File
# Begin Source File

SOURCE=.\misc\config_file\config_file.lex
# End Source File
# Begin Source File

SOURCE=.\misc\config_file\config_file.lex.cpp
# SUBTRACT CPP /YX /Yc /Yu
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

!ELSEIF  "$(CFG)" == "misc - Win32 DebugFast"

# PROP BASE Exclude_From_Build 1
# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\misc\config_file\config_file.yacc.cpp
# SUBTRACT CPP /YX /Yc /Yu
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
# Begin Source File

SOURCE=.\misc\win_event_emitter.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\win_event_emitter.h
# End Source File
# End Group
# Begin Group "Memory"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\misc\block_memory.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\block_memory.h
# End Source File
# Begin Source File

SOURCE=.\misc\heap_memory.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\heap_memory.h
# End Source File
# Begin Source File

SOURCE=.\misc\object_vector.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\object_vector.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\pool_memory.h
# End Source File
# Begin Source File

SOURCE=.\misc\stl_block_allocator.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\stl_block_allocator.h
# End Source File
# Begin Source File

SOURCE=.\misc\stl_block_list.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\stl_block_list.h
# End Source File
# End Group
# Begin Group "Makefile"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\misc\Makefile.am
# End Source File
# End Group
# Begin Group "Displayer"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\misc\debug_displayer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\debug_displayer.h
# End Source File
# Begin Source File

SOURCE=.\misc\displayer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\displayer.h
# End Source File
# Begin Source File

SOURCE=.\misc\gtk_displayer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\gtk_displayer.h
# End Source File
# Begin Source File

SOURCE=.\misc\log.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\log.h
# End Source File
# Begin Source File

SOURCE=.\misc\win_displayer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\win_displayer.h
# End Source File
# Begin Source File

SOURCE=.\misc\window_displayer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\window_displayer.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\misc\bit_set.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\bit_set.h
# End Source File
# Begin Source File

SOURCE=.\misc\bitmap.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\bitmap.h
# End Source File
# Begin Source File

SOURCE=.\misc\buf_fifo.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\buf_fifo.h
# End Source File
# Begin Source File

SOURCE=.\misc\class_id.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\class_id.h
# End Source File
# Begin Source File

SOURCE=.\misc\command.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\command.h
# End Source File
# Begin Source File

SOURCE=.\misc\common.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\common.h
# End Source File
# Begin Source File

SOURCE=.\misc\cpu_info.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\cpu_info.h
# End Source File
# Begin Source File

SOURCE=.\misc\debug.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\debug.h
# End Source File
# Begin Source File

SOURCE=.\misc\entity_id.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\entity_id.h
# End Source File
# Begin Source File

SOURCE=.\misc\i18n.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\i18n.h
# End Source File
# Begin Source File

SOURCE=.\misc\mutex.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\mutex.h
# End Source File
# Begin Source File

SOURCE=.\misc\p_thread.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\p_thread.h
# End Source File
# Begin Source File

SOURCE=.\misc\path.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\path.h
# End Source File
# Begin Source File

SOURCE=.\misc\quad.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\quad.h
# End Source File
# Begin Source File

SOURCE=.\misc\reader_writer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\reader_writer.h
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

SOURCE=..\include\nel\misc\smart_ptr_inline.h
# End Source File
# Begin Source File

SOURCE=.\misc\stdmisc.cpp
# ADD CPP /Yc"stdmisc.h"
# End Source File
# Begin Source File

SOURCE=.\misc\stdmisc.h
# End Source File
# Begin Source File

SOURCE=.\misc\stop_watch.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\stop_watch.h
# End Source File
# Begin Source File

SOURCE=.\misc\string_id_array.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\string_id_array.h
# End Source File
# Begin Source File

SOURCE=.\misc\system_info.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\system_info.h
# End Source File
# Begin Source File

SOURCE=.\misc\task_manager.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\task_manager.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\thread.h
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
# Begin Source File

SOURCE=..\include\nel\misc\ucstring.h
# End Source File
# Begin Source File

SOURCE=.\misc\value_smoother.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\value_smoother.h
# End Source File
# Begin Source File

SOURCE=.\misc\win_thread.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\misc\win_thread.h
# End Source File
# End Target
# End Project
