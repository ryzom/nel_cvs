#
# $Id: Makefile.am,v 1.2 2002/03/19 17:49:50 cado Exp $
#

MAINTAINERCLEANFILES  = Makefile.in

lib_LTLIBRARIES       = libnelgeorges.la

libnelgeorges_la_SOURCES =     form.cpp                  \
			       form_body_elt.cpp                  \
			       form_body_elt_atom.cpp                  \
			       form_body_elt_list.cpp                  \
			       form_body_elt_struct.cpp                  \
			       form_file.cpp                  \
			       form_head.cpp                  \
			       form_loader.cpp                  \
			       georges_loader.cpp                  \
			       item.cpp                  \
			       item_elt.cpp                  \
			       item_elt_atom.cpp                  \
			       item_elt_list.cpp                  \
			       item_elt_struct.cpp                  \
			       item_loader.cpp                  \
			       mold_elt.cpp                  \
			       mold_elt_define.cpp                  \
			       mold_elt_define_list.cpp                  \
			       mold_elt_type.cpp                  \
			       mold_elt_type_list.cpp                  \
			       mold_loader.cpp                  \
			       stdgeorges.cpp                  \
			       stdgeorges.h                  \
			       string_ex.cpp                  \
			       type_unit.cpp                  \
			       type_unit_double.cpp                  \
			       type_unit_file_name.cpp                  \
			       type_unit_int_signed.cpp                 \
			       type_unit_int_unsigned.cpp                  \
			       type_unit_string.cpp

AM_CXXFLAGS           = @XML_CFLAGS@

libnelgeorges_la_LIBADD  = -lc -lpthread @XML_LIBS@

libnelgeorges_la_LDFLAGS = -version-info @LIBTOOL_VERSION@


# End of Makefile.am

