#
# $Id: Makefile.am,v 1.16 2002/03/19 17:42:48 valignat Exp $
#

MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES      = libnelnet.la

libnelnet_la_SOURCES = buf_client.cpp                      \
                       buf_net_base.cpp                    \
                       buf_server.cpp                      \
                       buf_sock.cpp                        \
                       callback_client.cpp                 \
                       callback_net_base.cpp               \
                       callback_server.cpp                 \
                       dummy_tcp_sock.cpp                  \
                       inet_address.cpp                    \
                       listen_sock.cpp                     \
                       login_client.cpp                    \
                       login_cookie.cpp                    \
                       login_server.cpp                    \
                       message.cpp                         \
                       message_recorder.cpp                \
                       naming_client.cpp                   \
                       net_displayer.cpp                   \
                       net_log.cpp                         \
                       net_manager.cpp                     \
                       service.cpp                         \
                       sock.cpp                            \
                       stream_client.cpp                   \
                       stream_server.cpp                   \
                       tcp_sock.cpp                        \
                       udp_sock.cpp                        \
                       unitime.cpp                         \
                       unified_network.cpp                 \
                       service_5.cpp                       \
                       transport_class.cpp                 

noinst_HEADERS	     = stdnet.h

AM_CXXFLAGS          = -I$(top_srcdir)/src

libnelnet_la_LIBADD  = -lc -lpthread

libnelnet_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

