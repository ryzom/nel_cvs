#
# $Id: Makefile.am,v 1.7 2001/02/23 12:52:22 cado Exp $
#

MAINTAINERCLEANFILES = Makefile.in

lib_LTLIBRARIES      = libnelnet.la

libnelnet_la_SOURCES = base_socket.cpp cubic_entity_interpolator.cpp \
                       inet_address.cpp linear_entity_interpolator.cpp \
                       local_area.cpp local_entity.cpp message.cpp \
                       moving_entity.cpp msg_socket.cpp naming_client.cpp \
                       net_displayer.cpp net_log.cpp pt_callback_item.cpp \
                       remote_entity.cpp replica.cpp service.cpp socket.cpp \
                       unitime.cpp callback_net_base.cpp callback_client.cpp \
                       callback_server.cpp new_service.cpp

libnelnet_la_LIBADD  = -lc

libnelnet_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

