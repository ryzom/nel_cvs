# Microsoft Developer Studio Project File - Name="sound" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=sound - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "sound.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "sound.mak" CFG="sound - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "sound - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "sound - Win32 Debug" (based on "Win32 (x86) Static Library")
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
# ADD LIB32 /nologo

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
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "__STL_DEBUG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo

!ENDIF 

# Begin Target

# Name "sound - Win32 Release"
# Name "sound - Win32 Debug"
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

SOURCE=.\sound\env_effect.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\env_effect.h
# End Source File
# Begin Source File

SOURCE=.\sound\sound.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\sound.h
# End Source File
# Begin Source File

SOURCE=.\sound\track.cpp
# End Source File
# Begin Source File

SOURCE=.\sound\track.h
# End Source File
# End Target
# End Project
