#
# $Id: Makefile.am,v 1.5 2003/08/27 16:16:25 distrib Exp $
#

MAINTAINERCLEANFILES  = Makefile.in

lib_LTLIBRARIES       = libnelmemory.la

libnelmemory_la_SOURCES = 

#libnelmemory_la_SOURCES = heap_allocator.cpp          \
#			memory.cpp                  \
#			memory_common.cpp           \
#			memory_manager.cpp          \
#			memory_mutex.cpp            \
#			memory_tds.cpp

noinst_HEADERS        = heap_allocator.h            \
                        heap_allocator_inline.h     \
                        memory_common.h             \
                        memory_mutex.h              \
                        memory_tds.h

AM_CXXFLAGS           = -I$(top_srcdir)/src

libnelmemory_la_LIBADD  = -lc -lpthread

libnelmemory_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

