# Microsoft Developer Studio Project File - Name="net" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=net - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "net.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "net.mak" CFG="net - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "net - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "net - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "net - Win32 ReleaseDebug" (based on "Win32 (x86) Static Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "net - Win32 Release"

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
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/nlnet.lib"

!ELSEIF  "$(CFG)" == "net - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GR /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /D "__STL_DEBUG" /YX /FD /GZ /c
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo
# ADD LIB32 /nologo /out:"../lib/nlnet_debug.lib"

!ELSEIF  "$(CFG)" == "net - Win32 ReleaseDebug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "net___Win32_ReleaseDebug"
# PROP BASE Intermediate_Dir "net___Win32_ReleaseDebug"
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
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"../lib/nlnet.lib"
# ADD LIB32 /nologo /out:"../lib/nlnet_rd.lib"

!ENDIF 

# Begin Target

# Name "net - Win32 Release"
# Name "net - Win32 Debug"
# Name "net - Win32 ReleaseDebug"
# Begin Group "network_engine"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\net\base_socket.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\base_socket.h
# End Source File
# Begin Source File

SOURCE=.\net\inet_address.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\inet_address.h
# End Source File
# Begin Source File

SOURCE=.\net\message.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\message.h
# End Source File
# Begin Source File

SOURCE=.\net\msg_socket.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\msg_socket.h
# End Source File
# Begin Source File

SOURCE=.\net\pt_callback_item.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\pt_callback_item.h
# End Source File
# Begin Source File

SOURCE=.\net\socket.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\socket.h
# End Source File
# End Group
# Begin Group "dead_reckoning"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\net\cubic_entity_interpolator.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\cubic_entity_interpolator.h
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\entity_interpolator.h
# End Source File
# Begin Source File

SOURCE=.\net\linear_entity_interpolator.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\linear_entity_interpolator.h
# End Source File
# Begin Source File

SOURCE=.\net\local_area.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\local_area.h
# End Source File
# Begin Source File

SOURCE=.\net\local_entity.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\local_entity.h
# End Source File
# Begin Source File

SOURCE=.\net\moving_entity.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\moving_entity.h
# End Source File
# Begin Source File

SOURCE=.\net\remote_entity.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\remote_entity.h
# End Source File
# Begin Source File

SOURCE=.\net\replica.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\replica.h
# End Source File
# End Group
# Begin Group "new_network_engine"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\net\callback_client.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\callback_client.h
# End Source File
# Begin Source File

SOURCE=.\net\callback_net_base.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\callback_net_base.h
# End Source File
# Begin Source File

SOURCE=.\net\callback_server.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\callback_server.h
# End Source File
# Begin Source File

SOURCE=.\net\new_message.cpp

!IF  "$(CFG)" == "net - Win32 Release"

!ELSEIF  "$(CFG)" == "net - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "net - Win32 ReleaseDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\nel\net\new_message.h
# End Source File
# End Group
# Begin Group "service_clients"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\net\naming_client.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\naming_client.h
# End Source File
# Begin Source File

SOURCE=.\net\net_displayer.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\net_displayer.h
# End Source File
# Begin Source File

SOURCE=.\net\net_log.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\net_log.h
# End Source File
# Begin Source File

SOURCE=.\net\unitime.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\unitime.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\net\new_service.cpp

!IF  "$(CFG)" == "net - Win32 Release"

!ELSEIF  "$(CFG)" == "net - Win32 Debug"

!ELSEIF  "$(CFG)" == "net - Win32 ReleaseDebug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\include\nel\net\new_service.h
# End Source File
# Begin Source File

SOURCE=.\net\service.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\service.h
# End Source File
# End Target
# End Project
