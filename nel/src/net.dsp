# Microsoft Developer Studio Project File - Name="net" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Static Library" 0x0104

CFG=net - Win32 DebugFast
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "net.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "net.mak" CFG="net - Win32 DebugFast"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "net - Win32 Release" (based on "Win32 (x86) Static Library")
!MESSAGE "net - Win32 Debug" (based on "Win32 (x86) Static Library")
!MESSAGE "net - Win32 ReleaseDebug" (based on "Win32 (x86) Static Library")
!MESSAGE "net - Win32 DebugFast" (based on "Win32 (x86) Static Library")
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
# PROP Output_Dir "../obj/Release"
# PROP Intermediate_Dir "../obj/Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /Yu"stdnet.h" /FD /c
# SUBTRACT CPP /Fr
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
# PROP Output_Dir "../obj/Debug"
# PROP Intermediate_Dir "../obj/Debug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "_LIB" /YX /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /Zi /Od /Gf /D "_LIB" /D "_MBCS" /D "__STL_DEBUG" /D "WIN32" /D "_DEBUG" /Fr /Yu"stdnet.h" /FD /GZ /c
# SUBTRACT CPP /Gy
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
# PROP Output_Dir "../obj/ReleaseDebug"
# PROP Intermediate_Dir "../obj/ReleaseDebug"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GR /GX /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /YX /FD /c
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "WIN32" /D "NDEBUG" /D "_MBCS" /D "_LIB" /D "NL_RELEASE_DEBUG" /Yu"stdnet.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x40c /d "NDEBUG"
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"../lib/nlnet.lib"
# ADD LIB32 /nologo /out:"../lib/nlnet_rd.lib"

!ELSEIF  "$(CFG)" == "net - Win32 DebugFast"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "net___Win32_DebugFast"
# PROP BASE Intermediate_Dir "net___Win32_DebugFast"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../obj/DebugFast"
# PROP Intermediate_Dir "../obj/DebugFast"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /Zi /Od /Gf /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "__STL_DEBUG" /Fr /FD /GZ /c
# SUBTRACT BASE CPP /Gy /YX
# ADD CPP /nologo /MDd /W3 /Gm /Gi /GR /GX /Zi /Od /Ob1 /Gf /D "_LIB" /D "WIN32" /D "_DEBUG" /D "_MBCS" /D "NL_DEBUG_FAST" /Fr /Yu"stdnet.h" /FD /GZ /c
# SUBTRACT CPP /Gy
# ADD BASE RSC /l 0x40c /d "_DEBUG"
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD BASE LIB32 /nologo /out:"../lib/nlnet_debug.lib"
# ADD LIB32 /nologo /out:"../lib/nlnet_debug_fast.lib"

!ENDIF 

# Begin Target

# Name "net - Win32 Release"
# Name "net - Win32 Debug"
# Name "net - Win32 ReleaseDebug"
# Name "net - Win32 DebugFast"
# Begin Group "Layer0"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\net\dummy_tcp_sock.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\dummy_tcp_sock.h
# End Source File
# Begin Source File

SOURCE=.\net\inet_address.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\inet_address.h
# End Source File
# Begin Source File

SOURCE=.\net\listen_sock.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\listen_sock.h
# End Source File
# Begin Source File

SOURCE=.\net\sock.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\sock.h
# End Source File
# Begin Source File

SOURCE=.\net\tcp_sock.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\tcp_sock.h
# End Source File
# Begin Source File

SOURCE=.\net\udp_sock.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\udp_sock.h
# End Source File
# End Group
# Begin Group "Layer1"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\net\buf_client.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\buf_client.h
# End Source File
# Begin Source File

SOURCE=.\net\buf_net_base.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\buf_net_base.h
# End Source File
# Begin Source File

SOURCE=.\net\buf_server.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\buf_server.h
# End Source File
# Begin Source File

SOURCE=.\net\buf_sock.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\buf_sock.h
# End Source File
# End Group
# Begin Group "Layer2"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\net\stream_client.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\stream_client.h
# End Source File
# Begin Source File

SOURCE=.\net\stream_server.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\stream_server.h
# End Source File
# End Group
# Begin Group "Layer3"

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

SOURCE=.\net\message_recorder.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\message_recorder.h
# End Source File
# End Group
# Begin Group "Layer4"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\net\net_manager.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\net_manager.h
# End Source File
# Begin Source File

SOURCE=.\net\service.cpp
# End Source File
# Begin Source File

SOURCE=..\doc\net\service.dxt
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\service.h
# End Source File
# End Group
# Begin Group "Layer5"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\net\service_5.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\service_5.h
# End Source File
# Begin Source File

SOURCE=.\net\unified_network.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\unified_network.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\net\login_client.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\login_client.h
# End Source File
# Begin Source File

SOURCE=.\net\login_cookie.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\login_cookie.h
# End Source File
# Begin Source File

SOURCE=.\net\login_server.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\login_server.h
# End Source File
# Begin Source File

SOURCE=.\net\message.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\message.h
# End Source File
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

SOURCE=..\include\nel\net\pacs_client.h
# End Source File
# Begin Source File

SOURCE=.\net\stdnet.cpp
# ADD CPP /Yc"stdnet.h"
# End Source File
# Begin Source File

SOURCE=.\net\stdnet.h
# End Source File
# Begin Source File

SOURCE=.\net\transport_class.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\transport_class.h
# End Source File
# Begin Source File

SOURCE=.\net\unitime.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\unitime.h
# End Source File
# End Target
# End Project
