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

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../obj/Release/net"
# PROP Intermediate_Dir "../obj/Release/net"
# PROP Target_Dir ""
# ADD CPP /nologo /MD /W3 /GR /GX /O2 /D "NDEBUG" /D "_LIB" /D "WIN32" /D "_MBCS" /D "LIBXML_STATIC" /Yu"stdnet.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x40c
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /nologo /out:"../lib/nlnet_r.lib"

!ELSEIF  "$(CFG)" == "net - Win32 Debug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../obj/Debug/net"
# PROP Intermediate_Dir "../obj/Debug/net"
# PROP Target_Dir ""
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /D "_LIB" /D "__STL_DEBUG" /D "_DEBUG" /D "WIN32" /D "_MBCS" /D "LIBXML_STATIC" /Yu"stdnet.h" /FD /GZ /c
# SUBTRACT CPP /Gf /Gy /Fr
# ADD BASE RSC /l 0x40c
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /nologo /out:"../lib/nlnet_d.lib"

!ELSEIF  "$(CFG)" == "net - Win32 ReleaseDebug"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "../obj/ReleaseDebug/net"
# PROP Intermediate_Dir "../obj/ReleaseDebug/net"
# PROP Target_Dir ""
# ADD CPP /nologo /MD /W3 /GR /GX /Zi /O2 /D "NDEBUG" /D "_LIB" /D "NL_RELEASE_DEBUG" /D "WIN32" /D "_MBCS" /D "LIBXML_STATIC" /Yu"stdnet.h" /FD /c
# SUBTRACT CPP /Fr
# ADD BASE RSC /l 0x40c
# ADD RSC /l 0x40c /d "NDEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /nologo /out:"../lib/nlnet_rd.lib"

!ELSEIF  "$(CFG)" == "net - Win32 DebugFast"

# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "../obj/DebugFast/net"
# PROP Intermediate_Dir "../obj/DebugFast/net"
# PROP Target_Dir ""
# ADD CPP /nologo /MDd /W3 /GR /GX /Zi /Od /Ob1 /D "_LIB" /D "_DEBUG" /D "NL_DEBUG_FAST" /D "WIN32" /D "_MBCS" /D "LIBXML_STATIC" /Yu"stdnet.h" /FD /GZ /c
# SUBTRACT CPP /Gf /Gy /Fr
# ADD BASE RSC /l 0x40c
# ADD RSC /l 0x40c /d "_DEBUG"
BSC32=bscmake.exe
# ADD BSC32 /nologo
LIB32=link.exe -lib
# ADD LIB32 /nologo /out:"../lib/nlnet_df.lib"

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

SOURCE=.\net\udp_sim_sock.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\udp_sim_sock.h
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
# Begin Group "Layer5"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\net\unified_network.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\unified_network.h
# End Source File
# End Group
# Begin Source File

SOURCE=.\net\alarms.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\alarms.h
# End Source File
# Begin Source File

SOURCE=.\net\email.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\email.h
# End Source File
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

SOURCE=.\net\service.cpp
# End Source File
# Begin Source File

SOURCE=..\doc\net\service.dxt
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\service.h
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
# Begin Source File

SOURCE=.\net\varpath.cpp
# End Source File
# Begin Source File

SOURCE=..\include\nel\net\varpath.h
# End Source File
# End Target
# End Project
