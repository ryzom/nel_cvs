#
# $Id: Makefile.am,v 1.1 2002/11/15 17:36:05 lecroart Exp $
#

MAINTAINERCLEANFILES  = Makefile.in

SUBDIRS               = config_file

lib_LTLIBRARIES       = libnelmemory.la

libnelmisc_la_SOURCES = heap_allocator.cpp          \
			memory.cpp                  \
			memory_common.cpp           \
			memory_manager.cpp          \
			memory_mutex.cpp            \
			memory_tds.cpp

noinst_HEADERS        = heap_allocator.h            \
                        heap_allocator_inline.h     \
                        memory_common.h             \
                        memory_config.h             \
                        memory_mutex.h              \
                        memory_tds.h

libnelmisc_la_LIBADD  = -lc -lpthread

libnelmisc_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

