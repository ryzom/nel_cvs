# Microsoft Developer Studio Project File - Name="sound" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sound - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sound.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sound.mak" CFG="sound - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sound - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sound - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "sound - Win32 ReleaseDebug" (based on "Win32 (x86) Static Library")
!MESSAGE "sound - Win32 DebugFast" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "sound - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdsound.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/nlsound.lib"

!ELSEIF  "$(CFG)" == "sound - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /Zi /Od /Gf /D "_LIB" /D "__STL_DEBUG" /D "_MBCS" /D "WIN32" /D "_DEBUG" /Yu"stdsound.h" /FD /GZ /c
# SUBTRACT CPP /Gy /Fr
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/nlsound_debug.lib"

!ELSEIF  "$(CFG)" == "sound - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "sound___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "sound___Win32_ReleaseDebug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../obj/ReleaseDebug"
# PROP Intermediate_Dir "../obj/ReleaseDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NL_RELEASE_DEBUG" /Yu"stdsound.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/nlsound_rd.lib"

!ELSEIF  "$(CFG)" == "sound - Win32 DebugFast"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "sound___Win32_DebugFast"
# PROP BASE Intermediate_Dir "sound___Win32_DebugFast"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../obj/DebugFast"
# PROP Intermediate_Dir "../obj/DebugFast"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /Zi /Od /Gf /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "__STL_DEBUG" /Fr /FD /GZ /c
# SUBTRACT BASE CPP /Gy /YX
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /Zi /Od /Ob1 /Gf /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "NL_DEBUG_FAST" /Fr /Yu"stdsound.h" /FD /GZ /c
# SUBTRACT CPP /Gy
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"../lib/nlsound_debug.lib"
# ADD LIB32 /nologo /out:"../lib/nlsound_debug_fast.lib"

!ENDIF 

# Begin Target

# Name "sound - Win32 Release"
# Name "sound - Win32 Debug"
# Name "sound - Win32 ReleaseDebug"
# Name "sound - Win32 DebugFast"
# Begin Group "User"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\include\nel\sound\u_audio_mixer.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\sound\u_env_sound.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\sound\u_listener.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\sound\u_source.h
# End Source File
# End Group
# Begin Group "UserImp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\sound\audio_mixer_user.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\audio_mixer_user.h
# End Source File
# Begin Source File

SOURCE=.\sound\env_sound_user.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\env_sound_user.h
# End Source File
# Begin Source File

SOURCE=.\sound\listener_user.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\listener_user.h
# End Source File
# Begin Source File

SOURCE=.\sound\source_user.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\source_user.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\sound\ambiant_source.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\ambiant_source.h
# End Source File
# Begin Source File

SOURCE=.\sound\bounding_box.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\bounding_box.h
# End Source File
# Begin Source File

SOURCE=.\sound\bounding_shape.h
# End Source File
# Begin Source File

SOURCE=.\sound\bounding_sphere.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\bounding_sphere.h
# End Source File
# Begin Source File

SOURCE=.\sound\env_effect.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\env_effect.h
# End Source File
# Begin Source File

SOURCE=.\sound\mixing_track.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\mixing_track.h
# End Source File
# Begin Source File

SOURCE=.\sound\playable.h
# End Source File
# Begin Source File

SOURCE=.\sound\sample_bank.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\sample_bank.h
# End Source File
# Begin Source File

SOURCE=.\sound\sound.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\sound.h
# End Source File
# Begin Source File

SOURCE=.\sound\sound_bank.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\sound_bank.h
# End Source File
# Begin Source File

SOURCE=.\sound\stdsound.cpp
# ADD CPP /Yc"stdsound.h"
# End Source File
# Begin Source File

SOURCE=.\sound\stdsound.h
# End Source File
# Begin Source File

SOURCE=..\obj\DebugFast\nlsound_lowlevel_debug_fast.lib

!IF  "$(CFG)" == "sound - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "sound - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "sound - Win32 ReleaseDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "sound - Win32 DebugFast"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\obj\Release\nlsound_lowlevel.lib

!IF  "$(CFG)" == "sound - Win32 Release"

!ELSEIF  "$(CFG)" == "sound - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "sound - Win32 ReleaseDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "sound - Win32 DebugFast"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\obj\Debug\nlsound_lowlevel_debug.lib

!IF  "$(CFG)" == "sound - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "sound - Win32 Debug"

!ELSEIF  "$(CFG)" == "sound - Win32 ReleaseDebug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "sound - Win32 DebugFast"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\obj\ReleaseDebug\nlsound_lowlevel_rd.lib

!IF  "$(CFG)" == "sound - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "sound - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "sound - Win32 ReleaseDebug"

!ELSEIF  "$(CFG)" == "sound - Win32 DebugFast"

# PROP Exclude_From_Build 1

!ENDIF 

# End Source File
# End Target
# End Project
