/** \file types_nl.h
 * basic types, define and class
 *
 * $Id: types_nl.h,v 1.12 2000/11/09 16:17:03 coutelas Exp $
 *
 * \todo ace: create the ucstring type (unicode string type based on a STL basic_string?)
 *
 * Available constantes:
 * - NL_OS_WINDOWS		: windows operating system (32bits)
 * - NL_OS_UNIX			: linux operating system
 *
 * - NL_BIG_ENDIAN		: other processor
 * - NL_LITTLE_ENDIAN	: x86 processor
 *
 * - NL_DEBUG			: no optimization, full debug information, all log for the client
 * - NL_RELEASE			: full optimization, no debug information, no log for the client
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_TYPES_H
#define NL_TYPES_H


#include	<exception>

// Operating systems definition

#ifdef WIN32
#  define NL_OS_WINDOWS
#  define NL_LITTLE_ENDIAN
#  ifdef _DEBUG
#    define NL_DEBUG
#  else
#    define NL_RELEASE
#  endif
#else
#  define NL_OS_UNIX
#  define NL_BIG_ENDIAN
#endif

// Stupid Visual C++ warning

#ifdef NL_OS_WINDOWS
#  pragma warning (disable : 4503)			// STL: Decorated name length exceeded, name was truncated
#  pragma warning (disable : 4786)			// STL: too long indentifier
#  pragma warning (disable : 4290)			// throw() not implemented warning
#endif // NL_OS_WINDOWS

// Standard types

/*
 * correct numeric types:	sint8, uint8, sint16, uint16, sint32, uint32, sint64, uint64, sint, uint
 * correct char types:		char, string, wchar, wstring
 * correct misc types:		void, bool, float, double
 *
 */

/**
 * \typedef uint8
 * An unsigned 8 bits integer (use char only as \b character and not as integer)
 **/

/**
 * \typedef sint8
 * An signed 8 bits integer (use char only as \b character and not as integer)
 */

/**
 * \typedef uint16
 * An unsigned 16 bits integer (don't use short)
 **/

/**
 * \typedef sint16
 * An signed 16 bits integer (don't use short)
 */

/**
 * \typedef uint32
 * An unsigned 32 bits integer (don't use int or long)
 **/

/**
 * \typedef sint32
 * An signed 32 bits integer (don't use int or long)
 */

/**
 * \typedef uint64
 * An unsigned 64 bits integer (don't use long long or __int64)
 **/

/**
 * \typedef sint64
 * An signed 64 bits integer (don't use long long or __int64)
 */

/**
 * \typedef uint
 * An unsigned integer, at least 32 bits (used only for interal loops or speedy purpose, processor dependant)
 **/

/**
 * \typedef sint
 * An signed integer at least 32 bits (used only for interal loops or speedy purpose, processor dependant)
 */

/**
 * \def NL_I64
 * Used to display a int64 in a platform independant way with printf.
 \code
 printf("the value is: %"NL_I64"d\n"n, int64value);
 \endcode
 */

/**
 * \typedef ucchar
 * An unicode character (16 bits)
 */

#ifdef NL_OS_WINDOWS

typedef	signed		__int8		sint8;
typedef	unsigned	__int8		uint8;
typedef	signed		__int16		sint16;
typedef	unsigned	__int16		uint16;
typedef	signed		__int32		sint32;
typedef	unsigned	__int32		uint32;
typedef	signed		__int64		sint64;
typedef	unsigned	__int64		uint64;

typedef	signed		int			sint;			// at least 32bits (depend of processor)
typedef	unsigned	int			uint;			// at least 32bits (depend of processor)

#define	NL_I64	\
		"I64"

#elif defined (NL_OS_UNIX)

typedef	signed		char		sint8;
typedef	unsigned	char		uint8;
typedef	signed		short		sint16;
typedef	unsigned	short		uint16;
typedef	signed		int			sint32;
typedef	unsigned	int			uint32;
typedef	signed		long long	sint64;
typedef	unsigned	long long	uint64;

typedef	signed		int			sint;			// at least 32bits (depend of processor)
typedef	unsigned	int			uint;			// at least 32bits (depend of processor)

#define	NL_I64	\
		"ll"

#endif

typedef	uint16	ucchar;

// TODO: ucstring

/*
 * base class for all exceptions
 * possibility to add new functionnalities and behaviors.
 */
class Exception : public std::exception
{
};


#endif // NL_TYPES_H
