dnl =========================================================================
dnl
dnl Macros used by Nevrax in configure.in files.
dnl
dnl $Id: acinclude.m4,v 1.11 2002/03/26 14:37:45 valignat Exp $
dnl 
dnl =========================================================================

dnl =========================================================================
dnl WARNING: The original version of this file is placed in the $CVSROOT/code
dnl          directory.
dnl          There is links in the $CVSROOT/code sub-directories to that file
dnl          (ex: $CVSROOT/code/nel), so be careful of the consequences of
dnl          any modification of that file.
dnl =========================================================================

dnl =========================================================================
dnl Macros available in that file.
dnl
dnl
dnl AM_NEL_DEBUG
dnl
dnl    Option:      none.
dnl    Description: manage the different debug and the release mode by setting
dnl                 correctly the CFLAGS and CXXFLAGS variables.
dnl
dnl
dnl AM_PATH_NEL
dnl
dnl    Option:      none.
dnl    Description: check the instalation of the NeL library and set the
dnl                 CXXFLAGS and LIBS variables to use it.
dnl
dnl
dnl AM_PATH_STLPORT
dnl
dnl    Option:      none.
dnl    Description: check the instalation of the STLPort library and set the
dnl                 CXXFLAGS and LIBS variables to use it.
dnl
dnl
dnl AM_PATH_OPENGL
dnl
dnl    Option:      "yes" if the use of the OpenGL library is mandatory.
dnl    Description: check the instalation of the OpenGL library and set the
dnl                 OPENGL_CFLAGS and OPENGL_LIBS variables to use it.
dnl
dnl
dnl AM_PATH_FREETYPE
dnl
dnl    Option:      "yes" if the use of the Freetype library is mandatory.
dnl    Description: check the instalation of the OpenGL library and set the
dnl                 FREETYPE_CFLAGS and FREETYPE_LIBS variables to use it.
dnl
dnl
dnl AM_PATH_XF86VIDMODE
dnl
dnl    Option:      none.
dnl    Description: check the instalation of the OpenGL library and set the
dnl                 XF86VIDMODE_CFLAGS and XF86VIDMODE_LIBS variables to use it.
dnl
dnl
dnl AM_PATH_OPENAL
dnl
dnl    Option:      "yes" if the use of the OpenAL library is mandatory.
dnl    Description: check the instalation of the OpenGL library and set the
dnl                 OPENAL_CFLAGS and OPENAL_LIBS variables to use it.
dnl
dnl
dnl AM_PATH_PYTHON
dnl
dnl    Option:      "yes" if the use of the Python library is mandatory.
dnl    Description: check the instalation of the OpenGL library and set the
dnl                 PYTHON_CFLAGS and PYTHON_LIBS variables to use it.
dnl
dnl =========================================================================


# =========================================================================
# AM_NEL_DEBUG

AC_DEFUN(AM_NEL_DEBUG,
[

MAX_C_OPTIMIZE="-O6"

STL_DEBUG="-D__STL_DEBUG"

NL_DEBUG="-DNL_DEBUG"
NL_DEBUG_FAST="-DNL_DEBUG_FAST"
NL_RELEASE_DEBUG="-DNL_RELEASE_DEBUG"
NL_RELEASE="-DNL_RELEASE"

AC_ARG_WITH(debug,
    [  --with-debug[=full|medium|fast]
                          Build a debug version (huge libraries).
                          Full mode set only NeL and STL debug flags.
                          Medium mode set NeL debug flags with inline
                          optimization (default mode).
                          Fast mode is like the Medium mode with some basic
                          optimization.
  --without-debug         Build without debugging code (default)],
    [with_debug=$withval],
    [with_debug=no])

dnl Build optimized or debug version ?
dnl First check for gcc and g++
if test "$ac_cv_prog_gcc" = "yes"
then
    DEBUG_CFLAGS="-g"
    DEBUG_OPTIMIZE_CC="-O"
    OPTIMIZE_CFLAGS="$MAX_C_OPTIMIZE"
else
    DEBUG_CFLAGS="-g"
    DEBUG_OPTIMIZE_CC=""
    OPTIMIZE_CFLAGS="-O"
fi

if test "$ac_cv_prog_cxx_g" = "yes"
then
    DEBUG_CXXFLAGS="-g"
    DEBUG_OPTIMIZE_CXX="-O"
    OPTIMIZE_CXXFLAGS="-O3"
    OPTIMIZE_INLINE_CXXFLAGS="-finline-functions"
else
    DEBUG_CXXFLAGS="-g"
    DEBUG_OPTIMIZE_CXX=""
    OPTIMIZE_CXXFLAGS="-O"
    OPTIMIZE_INLINE_CXXFLAGS=""
fi

if test "$with_debug" = "yes" -o "$with_debug" = "medium"
then
    dnl Medium debug. Inline optimization
    CFLAGS="$DEBUG_CFLAGS $OPTIMIZE_INLINE_CFLAGS $NL_DEBUG $NL_DEBUG_FAST $CFLAGS"
    CXXFLAGS="$DEBUG_CXXFLAGS $OPTIMIZE_INLINE_CXXFLAGS $NL_DEBUG $NL_DEBUG_FAST $CXXFLAGS"
else
    if test "$with_debug" = "full"
    then
        dnl Full debug. Very slow in some cases
        CFLAGS="$DEBUG_CFLAGS $NL_DEBUG $STL_DEBUG $CFLAGS"
        CXXFLAGS="$DEBUG_CXXFLAGS $NL_DEBUG $STL_DEBUG $CXXFLAGS"
    else
        if test "$with_debug" = "fast"
        then
            dnl Fast debug.
            CFLAGS="$DEBUG_CFLAGS $DEBUG_OPTIMIZE_CC $OPTIMIZE_INLINE_CFLAGS $NL_DEBUG $CFLAGS"
            CXXFLAGS="$DEBUG_CXXFLAGS $DEBUG_OPTIMIZE_CXX $OPTIMIZE_INLINE_CXXFLAGS $NL_DEBUG $CXXFLAGS"
        else
            dnl Optimized version. No debug
            CFLAGS="$OPTIMIZE_CFLAGS $NL_RELEASE $CFLAGS"
            CXXFLAGS="$OPTIMIZE_CXXFLAGS $NL_RELEASE $CXXFLAGS"
        fi
    fi
fi

dnl AC_MSG_RESULT([CFLAGS = $CFLAGS])
dnl AC_MSG_RESULT([CXXGLAGS = $CXXFLAGS])

])


# =========================================================================
# MY_NEL_HEADER_CHK : NeL header files checking macros

AC_DEFUN(MY_NEL_HEADER_CHK,
[ AC_REQUIRE_CPP()

chk_message_obj="$1"
header="$2"
macro="$3"
is_mandatory="$4"

if test $is_mandatory = "yes"
then

    _CPPFLAGS="$CPPFLAGS"

    CPPFLAGS="$CXXFLAGS $NEL_CFLAGS"

    AC_MSG_CHECKING(for $header)

    AC_EGREP_CPP( yo_header,
[#include <$header>
#ifdef $macro
   yo_header
#endif],
  have_header="yes",
  have_header="no")

    CPPFLAGS="$_CPPFLAGS"

    if test "$have_header" = "yes"
    then
        AC_MSG_RESULT(yes)
    else
        if test "$is_mandatory" = "yes"
        then
            AC_MSG_ERROR([$chk_message_obj must be installed (http://www.nevrax.org).])
        else
            AC_MSG_RESULT(no)
        fi
    fi
fi

        
])


# =========================================================================
# MY_NEL_LIB_CHK : NeL library checking macros

AC_DEFUN(MY_NEL_LIB_CHK,
[ AC_REQUIRE_CPP()

chk_message_obj="$1"
nel_test_lib="$2"
is_mandatory="$3"

if test $is_mandatory = "yes"
then

    AC_CHECK_LIB($nel_test_lib, main,,[AC_MSG_ERROR([$chk_message_obj must be installed (http://www.nevrax.org).])])
fi
])


# =========================================================================
# AM_PATH_NEL : NeL checking macros
AC_DEFUN(AM_PATH_NEL,
[ AC_REQUIRE_CPP()

AC_ARG_WITH( nel,
    [  --with-nel=<path>       path to the NeL install files directory.
                          e.g. /usr/local/nel])

AC_ARG_WITH( nel-include,
    [  --with-nel-include=<path>
                          path to the NeL header files directory.
                          e.g. /usr/local/nel/stlport])

AC_ARG_WITH( nel-lib,
    [  --with-nel-lib=<path>
                          path to the NeL library files directory.
                          e.g. /usr/local/nel/lib])


nelmisc_is_mandatory="$1"
nelnet_is_mandatory="$2"
nel3d_is_mandatory="$3"
nelpacs_is_mandatory="$4"
nelsound_is_mandatory="$5"
nelai_is_mandatory="$6"
nelgeorges_is_mandatory="$7"

dnl Check for nel-config
AC_PATH_PROG(NEL_CONFIG, nel-config, no)

dnl 
dnl Configure options (--with-nel*) have precendence 
dnl over nel-config only set variables if they are not 
dnl specified
dnl
if test "$NEL_CONFIG" != "no"
then
    if test -z "$with_nel" -a -z "$with_nel_include"
    then
	CXXFLAGS="$CXXFLAGS `nel-config --cflags`"
    fi

    if test -z "$with_nel" -a -z "$with_nel_lib"
    then
	LDFLAGS="`nel-config --ldflags` $LDFLAGS"
    fi
fi

dnl
dnl Set nel_libraries and nel_includes according to
dnl user specification (--with-nel*) if any. 
dnl --with-nel-include and --with-nel-lib have precendence
dnl over --with-nel
dnl
if test "$with_nel" = "no"
then
    dnl The user explicitly disabled the use of the NeL
    AC_MSG_ERROR([NeL is mandatory: do not specify --without-nel])
else
    if test "$with_nel" -a "$with_nel" != "yes"
    then
	nel_includes="$with_nel/include"
	nel_libraries="$with_nel/lib"
    fi
fi

if test "$with_nel_include"
then
    nel_includes="$with_nel_include"
fi

if test "$with_nel_lib"
then
    nel_libraries="$with_nel_lib"
fi

dnl
dnl Set compilation variables 
dnl
if test "$nel_includes"
then
    CXXFLAGS="$CXXFLAGS -I$nel_includes"
fi

if test "$nel_libraries"
then
    LDFLAGS="-L$nel_libraries $LDFLAGS"
fi

dnl
dnl Collect headers information and bark if missing and
dnl mandatory
dnl

MY_NEL_HEADER_CHK([NeL Misc], [nel/misc/types_nl.h], [NL_TYPES_H], $nelmisc_is_mandatory)
MY_NEL_HEADER_CHK([NeL Network], [nel/net/sock.h], [NL_SOCK_H], $nelnet_is_mandatory)
MY_NEL_HEADER_CHK([NeL 3D], [nel/3d/u_camera.h], [NL_U_CAMERA_H], $nel3d_is_mandatory)
MY_NEL_HEADER_CHK([NeL PACS], [nel/pacs/u_global_position.h], [NL_U_GLOBAL_POSITION_H], $nelpacs_is_mandatory)
MY_NEL_HEADER_CHK([NeL Sound], [nel/sound/u_source.h], [NL_U_SOURCE_H], $nelsound_is_mandatory)
MY_NEL_HEADER_CHK([NeL AI], [nel/ai/nl_ai.h], [_IA_NEL_H], $nelai_is_mandatory)
MY_NEL_HEADER_CHK([NeL Georges], [nel/georges/common.h], [NLGEORGES_COMMON_H], $nelgeorges_is_mandatory)

dnl
dnl Collect libraries information and bark if missing and
dnl mandatory
dnl

MY_NEL_LIB_CHK([NeL Misc], [nelmisc], $nelmisc_is_mandatory)
MY_NEL_LIB_CHK([NeL Network], [nelnet], $nelnet_is_mandatory)
MY_NEL_LIB_CHK([NeL 3D], [nel3d], $nel3d_is_mandatory)
MY_NEL_LIB_CHK([NeL PACS], [nelpacs], $nelpacs_is_mandatory)
MY_NEL_LIB_CHK([NeL Sound], [nelsnd], $nelsound_is_mandatory)
MY_NEL_LIB_CHK([NeL AI], [nelai], $nelai_is_mandatory)
MY_NEL_LIB_CHK([NeL Georges], [nelgeorges], $nelgeorges_is_mandatory)

])


# =========================================================================
# AM_PATH_STLPORT : STLPort checking macros

AC_DEFUN(AM_PATH_STLPORT,
[ AC_REQUIRE_CPP()

AC_ARG_WITH( stlport,
    [  --with-stlport=<path>   path to the STLPort install files directory.
                          e.g. /usr/local/stlport])

AC_ARG_WITH( stlport-include,
    [  --with-stlport-include=<path>
                          path to the STLPort header files directory.
                          e.g. /usr/local/stlport/stlport])

AC_ARG_WITH( stlport-lib,
    [  --with-stlport-lib=<path>
                          path to the STLPort library files directory.
                          e.g. /usr/local/stlport/lib])

if test "$with_debug" = "full"
then
 stlport_lib="stlport_gcc_stldebug"
else
 stlport_lib="stlport_gcc"
fi

if test "$with_stlport" = no
then
    dnl The user explicitly disabled the use of the STLPorts
    AC_MSG_ERROR([STLPort is mandatory: do not specify --without-stlport])
else
    stlport_includes="/usr/include/stlport"
    if test "$with_stlport" -a "$with_stlport" != yes
    then
        stlport_includes="$with_stlport/stlport"
        stlport_libraries="$with_stlport/lib"

        if test ! -d "$stlport_includes"
        then
            stlport_includes="$with_stlport/include/stlport"
        fi
    fi
fi

if test "$with_stlport_include"
then
    stlport_includes="$with_stlport_include"
fi

if test "$with_stlport_lib"
then
    stlport_libraries="$with_stlport_lib"
fi

AC_LANG_SAVE
AC_LANG_CPLUSPLUS

dnl Put STLPorts includes in CXXFLAGS
if test "$stlport_includes"
then
    CXXFLAGS="$CXXFLAGS -I$stlport_includes"
fi

dnl Put STLPorts libraries directory in LIBS
if test "$stlport_libraries"
then
    LIBS="-L$stlport_libraries $LIBS"
else
    stlport_libraries='default'
fi

dnl Test the headers

AC_CHECK_HEADER(algorithm,
    have_stlport_headers="yes",
    have_stlport_headers="no" )

AC_MSG_CHECKING(for STLPort headers)

if test "$have_stlport_headers" = "yes"
then
    AC_MSG_RESULT([$stlport_includes])
else
    AC_MSG_RESULT(no)
fi

AC_CHECK_LIB($stlport_lib, main,, have_stlport_libraries="no")

AC_MSG_CHECKING(for STLPort library)

if test "$have_stlport_libraries" != "no"
then
    AC_MSG_RESULT([$stlport_libraries])
else
    AC_MSG_RESULT(no)
fi

if test "$have_stlport_headers" = "yes" &&
    test "$have_stlport_libraries" != "no"
then
    have_stlport="yes"
else
    have_stlport="no"
fi

if test "$have_stlport" = "no"
then
    AC_MSG_ERROR([STLPort must be installed (http://www.stlport.org).])
fi

AC_LANG_RESTORE

])


# =========================================================================
# AM_PATH_OPENGL : OpenGL checking macros

AC_DEFUN(AM_PATH_OPENGL,
[ AC_MSG_CHECKING(for OpenGL headers and GL Version >= 1.2)

is_mandatory="$1"

AC_REQUIRE_CPP()

AC_ARG_WITH( opengl,
    [  --with-opengl=<path>    path to the OpenGL install files directory.
                          e.g. /usr/local])

AC_ARG_WITH( opengl-include,
    [  --with-opengl-include=<path>
                          path to the OpenGL header files directory.
                          e.g. /usr/local/include])

AC_ARG_WITH( opengl-lib,
    [  --with-opengl-lib=<path>
                          path to the OpenGL library files directory.
                          e.g. /usr/local/lib])

opengl_lib="GL"

if test "$with_opengl"
then
    opengl_includes="$with_opengl/include"
    opengl_libraries="$with_opengl/lib"
fi

if test "$with_opengl_include"
then
    opengl_includes="$with_opengl_include"
fi

if test "$with_opengl_lib"
then
    opengl_libraries="$with_opengl_lib"
fi

dnl Set OPENGL_CFLAGS
if test "$opengl_includes"
then
    OPENGL_CFLAGS="-I$opengl_includes"
fi

dnl Set OPENGL_LIBS
if test "$opengl_libraries"
then
    OPENGL_LIBS="-L$opengl_libraries"
fi
OPENGL_LIBS="$OPENGL_LIBS -l$opengl_lib"

dnl Test the headers
_CPPFLAGS="$CPPFLAGS"

CPPFLAGS="$CXXFLAGS $OPENGL_CFLAGS"

AC_EGREP_CPP( yo_opengl,
[#include <GL/gl.h>       
#if defined(GL_VERSION_1_2)
   yo_opengl
#endif],
  have_opengl_headers="yes", 
  have_opengl_headers="no" )

if test "$have_opengl_headers" = "yes"
then
    if test "$opengl_includes"
    then
        AC_MSG_RESULT([$opengl_includes])
    else
        AC_MSG_RESULT(yes)
    fi
else
    AC_MSG_RESULT(no)
fi

dnl Checking the GLEXT version >= 7
AC_MSG_CHECKING(for <GL/glext.h> and GLEXT version >= 7)

AC_EGREP_CPP( yo_glext_version,
[#include <GL/glext.h>
#ifdef GL_GLEXT_VERSION
#if GL_GLEXT_VERSION >= 7
   yo_glext_version
#endif
#endif],
  have_glext="yes",
  have_glext="no" )

if test "$have_glext" = "yes"
then
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT([no, <GL/glext.h> can be downloaded from http://oss.sgi.com/projects/ogl-sample/ABI/])
fi
    
dnl Test the libraries
AC_MSG_CHECKING(for OpenGL libraries)

CPPFLAGS="$CXXFLAGS $OPENGL_LIBS"

AC_TRY_LINK( , , have_opengl_libraries="yes", have_opengl_libraries="no")

CPPFLAGS="$_CPPFLAGS"

if test "$have_opengl_libraries" = "yes"
then
    if test "$opengl_libraries"
    then
        AC_MSG_RESULT([$opengl_libraries])
    else
        AC_MSG_RESULT(yes)
    fi
else
    AC_MSG_RESULT(no)
fi

opengl_libraries="$opengl_libraries"

if test "$have_opengl_headers" = "yes" \
        -a "$have_glext" = "yes" \
        -a "$have_opengl_libraries" = "yes"
then
    have_opengl="yes"
else
    have_opengl="no"
fi

if test "$have_opengl" = "no" -a "$is_mandatory" = "yes"
then
    AC_MSG_ERROR([OpenGL >= 1.2 must be installed (http://www.mesa3d.org)])
fi

AC_SUBST(OPENGL_CFLAGS)
AC_SUBST(OPENGL_LIBS)

])


# =========================================================================
# AM_PATH_FREETYPE : FreeType checking macros

AC_DEFUN(AM_PATH_FREETYPE,
[ is_mandatory="$1"

AC_REQUIRE_CPP()

AC_ARG_WITH( freetype,
    [  --with-freetype=<path>   path to the FreeType install files directory.
                          e.g. /usr/local/freetype])

AC_ARG_WITH( freetype-include,
    [  --with-freetype-include=<path>
                          path to the FreeType header files directory.
                          e.g. /usr/local/freetype/include])

AC_ARG_WITH( freetype-lib,
    [  --with-freetype-lib=<path>
                          path to the FreeType library files directory.
                          e.g. /usr/local/freetype/lib])

freetype_lib="freetype"


AC_PATH_PROG(FREETYPE_CONFIG, freetype-config, no)
  
if test "$FREETYPE_CONFIG" = "no"
then
    have_freetype_config="no"
else
    FREETYPE_CFLAGS=`freetype-config --cflags`
    FREETYPE_LIBS=`freetype-config --libs`
    have_freetype_config="yes"
fi

if test "$with_freetype"
then
    freetype_includes="$with_freetype/include"
    freetype_libraries="$with_freetype/lib"
fi

if test "$with_freetype_include"
then
    freetype_includes="$with_freetype_include"
fi

if test "$with_freetype_lib"
then
    freetype_libraries="$with_freetype_lib"
fi

if test "$freetype_includes"
then
    FREETYPE_CFLAGS="-I$freetype_includes"
fi

dnl Checking the FreeType 2 instalation
_CPPFLAGS="$CPPFLAGS"
CPPFLAGS=" $FREETYPE_CFLAGS $CXXFLAGS"

AC_MSG_CHECKING(for FreeType version = 2)

AC_EGREP_CPP( yo_freetype2,
[#include <freetype/freetype.h>
#if FREETYPE_MAJOR == 2
   yo_freetype2
#endif],
  have_freetype2="yes",
  have_freetype2="no")

if test "$have_freetype2" = "yes"
then
    AC_MSG_RESULT(yes)
else
    AC_MSG_RESULT(no)
fi
    
dnl Test the libraries
AC_MSG_CHECKING(for FreeType libraries)
        
if test $freetype_libraries
then
    FREETYPE_LIBS="-L$freetype_libraries -l$freetype_lib"
fi

CPPFLAGS="$FREETYPE_LIBS $CXXFLAGS"
    
AC_TRY_LINK( , , have_freetype_libraries="yes", have_freetype_libraries="no")

CPPFLAGS="$_CPPFLAGS"

if test "$have_freetype_libraries" = "yes"
then    
    if test "$freetype_libraries"
    then
        AC_MSG_RESULT([$freetype_libraries])
    else
        AC_MSG_RESULT(yes)
    fi
else
    AC_MSG_RESULT(no)
fi

if test "$have_freetype2" = "yes" && test "$have_freetype_libraries" = "yes"
then
    have_freetype="yes"
else
    have_freetype="no"
fi

if test "$have_freetype" = "no" && test "$is_mandatory" = "yes"
then
    AC_MSG_ERROR([FreeType 2 must be installed (http://freetype.sourceforge.net)])
fi

AC_SUBST(FREETYPE_CFLAGS)
AC_SUBST(FREETYPE_LIBS)

])


# =========================================================================
# AM_PATH_XF86VIDMODE : XF86VidMode checking macros

AC_DEFUN(AM_PATH_XF86VIDMODE,
[ AC_MSG_CHECKING(for XF86VidMode extension)

AC_REQUIRE_CPP()

AC_ARG_WITH( xf86vidmode-lib,
    [  --with-xf86vidmode-lib=<path>
                          path to the XF86VidMode library.
                          e.g. /usr/X11R6/lib] )

xf86vidmode_lib="Xxf86vm"

if test "$with_xf86vidmode_lib" = no
then
    dnl The user explicitly disabled the use of XF86VidMode
    have_xf86vidmode="disabled"
    AC_MSG_RESULT(disabled)
else
    if test "$with_xf86vidmode_lib"
    then
        xf86vidmode_libraries="$with_xf86vidmode_lib"
    fi

    XF86VIDMODE_CFLAGS="-DXF86VIDMODE"
fi

if test -z "$have_xf86vidmode"
dnl -a "$with_xf86vidmode_lib"
then
    if test "$xf86vidmode_libraries"
    then
        XF86VIDMODE_LIBS="-L$xf86vidmode_libraries"
    fi

    XF86VIDMODE_LIBS="$XF86VIDMODE_LIBS -l$xf86vidmode_lib"

    _CPPFLAGS="$CPPFLAGS"

    CPPFLAGS="$CXXFLAGS $XF86VIDMODE_LIBS"

    AC_TRY_LINK( , , have_xf86vidmode_libraries="yes", have_xf86vidmode_libraries="no")

    CPPFLAGS="$_CPPFLAGS"

    if test "$have_xf86vidmode_libraries" = "yes"
    then
        have_xf86vidmode="yes"
        if test "$xf86vidmode_libraries"
        then
            AC_MSG_RESULT($xf86vidmode_libraries)
        else
            AC_MSG_RESULT(yes)
        fi
    else
        have_xf86vidmode="no"
        AC_MSG_RESULT(no, no fullscreen support available.)
    fi

    xf86vidmode_libraries="$xf86vidmode_libraries"

fi

AC_SUBST(XF86VIDMODE_CFLAGS)
AC_SUBST(XF86VIDMODE_LIBS)

])


# =========================================================================
# AM_PATH_OPENAL : OpenAL checking macros

AC_DEFUN(AM_PATH_OPENAL,
[ is_mandatory="$1"

AC_REQUIRE_CPP()

dnl Get from the user option the path to the OpenAL files location
AC_ARG_WITH( openal,
    [  --with-openal=<path>   path to the OpenAL install files directory.
                          e.g. /usr/local])

AC_ARG_WITH( openal-include,
    [  --with-openal-include=<path>
                          path to the OpenAL header files directory.
                          e.g. /usr/local/include])

AC_ARG_WITH( openal-lib,
    [  --with-openal-lib=<path>
                          path to the OpenAL library files directory.
                          e.g. /usr/local/lib])

openal_lib="openal"

if test $with_openal
then
    openal_includes="$with_openal/include"
    openal_libraries="$with_openal/lib"
fi

if test "$with_openal_include"
then
    openal_includes="$with_openal_include"
fi

if test "$with_openal_lib"
then
    openal_libraries="$with_openal_lib"
fi


dnl Set OPENAL_CFLAGS
if test "$openal_includes"
then
    OPENAL_CFLAGS="-I$openal_includes"
fi

dnl Set OPENAL_LIBS
if test "$openal_libraries"
then
    OPENAL_LIBS="-L$openal_libraries"
fi
OPENAL_LIBS="$OPENAL_LIBS -l$openal_lib"

_CPPFLAGS="$CPPFLAGS"
CPPFLAGS="$CXXFLAGS $OPENAL_CFLAGS"

AC_MSG_CHECKING(for OpenAL headers)
AC_EGREP_CPP( yo_openal,
[#include <AL/altypes.h>
#ifdef AL_VERSION
   yo_openal
#endif],
  have_openal_headers="yes",
  have_openal_headers="no" )

if test "$have_openal_headers" = "yes"
then
    if test "$openal_includes"
    then
        AC_MSG_RESULT([$openal_includes])
    else
        AC_MSG_RESULT(yes)
    fi
else
    AC_MSG_RESULT(no)
fi

dnl Test the libraries
AC_MSG_CHECKING(for OpenAL libraries)

CPPFLAGS="$CXXFLAGS $OPENAL_LIBS"

AC_TRY_LINK( , , have_openal_libraries="yes", have_openal_libraries="no")

CPPFLAGS="$_CPPFLAGS"

if test "$have_openal_libraries" = "yes"
then
    if test "$openal_libraries"
    then
        AC_MSG_RESULT([$openal_libraries])
    else
        AC_MSG_RESULT(yes)
    fi
else
    AC_MSG_RESULT(no)
fi

openal_libraries="$openal_libraries"

if test "$have_openal_headers" = "yes" \
   && test "$have_openal_libraries" = "yes"
then
    have_openal="yes"
else
    have_openal="no"
fi

if test "$have_openal" = "no" -a "$is_mandatory" = "yes"
then
    AC_MSG_ERROR([OpenAL is needed to compile NeL (http://www.openal.org).])
fi

AC_SUBST(OPENAL_CFLAGS)
AC_SUBST(OPENAL_LIBS)


])


# =========================================================================
# AM_PATH_PYTHON : Python checking macros

AC_DEFUN(AM_PATH_PYTHON,
[ python_version_required="$1"

is_mandatory="$2"

AC_REQUIRE_CPP()

dnl Get from the user option the path to the Python files location
AC_ARG_WITH( python,
    [  --with-python=<path>    path to the Python prefix installation directory.
                          e.g. /usr/local],
    [ PYTHON_PREFIX=$with_python ]
)

AC_ARG_WITH( python-version,
    [  --with-python-version=<version>
                          Python version to use, e.g. 1.5],
    [ PYTHON_VERSION=$with_python_version ]
)

if test ! "$PYTHON_PREFIX" = ""
then
    PATH="$PYTHON_PREFIX/bin:$PATH"
fi

if test ! "$PYTHON_VERSION" = ""
then
    PYTHON_EXEC="python$PYTHON_VERSION"
else
    PYTHON_EXEC="python python2.1 python2.0 python1.5"
fi

AC_PATH_PROGS(PYTHON, $PYTHON_EXEC, no, $PATH)

if test "$PYTHON" != "no"
then
    PYTHON_PREFIX=`$PYTHON -c 'import sys; print "%s" % (sys.prefix)'`
    PYTHON_VERSION=`$PYTHON -c 'import sys; print "%s" % (sys.version[[:3]])'`

    is_python_version_enough=`expr $python_version_required \<= $PYTHON_VERSION`
fi


if test "$PYTHON" = "no" || test "$is_python_version_enough" != "1"
then

    if test "$is_mandatory" = "yes"
    then
        AC_MSG_ERROR([Python $python_version_required must be installed (http://www.python.org)])
    else
        have_python="no"
    fi

else

    python_includes="$PYTHON_PREFIX/include/python$PYTHON_VERSION"
    python_libraries="$PYTHON_PREFIX/lib/python$PYTHON_VERSION/config"
    python_lib="python$PYTHON_VERSION"

    PYTHON_CFLAGS="-I$python_includes"
    PYTHON_LIBS="-L$python_libraries -l$python_lib"

    _CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$CXXFLAGS ${PYTHON_CFLAGS}"

    dnl Test the headers
    AC_MSG_CHECKING(for Python headers)

    AC_EGREP_CPP( yo_python,
    [#include <Python.h>
   yo_python
    ],
      have_python_headers="yes",
      have_python_headers="no" )

    if test "$have_python_headers" = "yes"
    then
        AC_MSG_RESULT([$python_includes])
    else
        AC_MSG_RESULT(no)
    fi

    dnl Test the libraries
    AC_MSG_CHECKING(for Python libraries)

    CPPFLAGS="$CXXFLAGS $PYTHON_CFLAGS"

    AC_TRY_LINK( , , have_python_libraries="yes", have_python_libraries="no")

    CPPFLAGS="$_CPPFLAGS"

    if test "$have_python_libraries" = "yes"
    then
        if test "$python_libraries"
        then
            AC_MSG_RESULT([$python_libraries])
        else
            AC_MSG_RESULT(yes)
        fi
    else
        AC_MSG_RESULT(no)
    fi

    if test "$have_python_headers" = "yes" \
       && test "$have_python_libraries" = "yes"
    then
        have_python="yes"
    else
        have_python="no"
    fi

    if test "$have_python" = "no" -a "$is_mandatory" = "yes"
    then
        AC_MSG_ERROR([Python is needed to compile NeL (http://www.python.org).])
    fi

    AC_SUBST(PYTHON_CFLAGS)
    AC_SUBST(PYTHON_LIBS)

fi

])


dnl =========================================================================
dnl End of file
