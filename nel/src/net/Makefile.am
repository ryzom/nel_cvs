#
# $Id: Makefile.am,v 1.9 2001/05/02 12:36:31 lecroart Exp $
#

MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES      = libnelnet.la

libnelnet_la_SOURCES = buf_client.cpp buf_net_base.cpp buf_server.cpp \
                       buf_sock.cpp callback_client.cpp callback_net_base.cpp \
                       callback_server.cpp inet_address.cpp listen_sock.cpp \
                       login_client.cpp login_cookie.cpp login_server.cpp \
                       message.cpp msg_client.cpp msg_net_base.cpp msg_server.cpp \
                       naming_client.cpp net_displayer.cpp net_log.cpp net_manager.cpp \
                       service.cpp sock.cpp stream_client.cpp stream_server.cpp \
                       tcp_sock.cpp udp_sock.cpp unitime.cpp             

libnelnet_la_LIBADD  = -lc

libnelnet_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

