
/*
 * Compilation mode :	DEBUG	: no optimization, full debug information, all log for the client
 *						BETA	: full optimization, full debug information, all log for the client
 *						RELEASE	: full optimization, no debug information, no log for the client
 */

/*
 * Evalable constantes :
 *	NL_OS_WINDOWS		: windows operating system (32bits)
 *	NL_OS_LINUX			: linux operating system
 *
 *	NL_BIG_ENDIAN		: other processor
 *	NL_LITTLE_ENDIAN	: x86 processor
 */


#ifndef NL_TYPES_H
#define NL_TYPES_H

// Operating systems

#ifdef WIN32
#  define NL_OS_WINDOWS
#  define NL_LITTLE_ENDIAN
#else
#  define NL_OS_LINUX
#  define NL_BIG_ENDIAN
#endif

// Stupid Visual C++ warning

#ifdef NL_OS_WINDOWS
#  pragma warning (disable : 4786)			// STL too long indentifier
#  pragma warning (disable : 4290)			// throw() not implemented warning
#endif

// Standard types

/*
 * correct numeric types:	sint8, uint8, sint16, uint16, sint32, uint32, sint64, uint64, sint, uint
 * correct char types:		char, string, wchar, wstring
 * correct misc types:		void, bool, float, double
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

typedef	signed		__int32		sint;			// at least 32bits (depend of processor)
typedef	unsigned	__int32		uint;			// at least 32bits (depend of processor)

#elif NL_OS_LINUX

typedef	signed		char		sint8;
typedef	unsigned	char		uint8;
typedef	signed		short		sint16;
typedef	unsigned	short		uint16;
typedef	signed		int			sint32;			// 32 bits exclusively!	(a discuter...)
typedef	unsigned	int			uint32;			// 32 bits exclusively!	(a discuter...)
typedef	signed		long long	sint64;
typedef	unsigned	long long	uint64;

typedef	signed		int			sint;			// at least 32bits (depend of processor)
typedef	unsigned	int			uint;			// at least 32bits (depend of processor)

#endif

typedef	unsigned	short		wchar;

// TODO: wstring

/*
 * classe de base a toutes les exceptions
 * permet d'ajouter de nouveaux comportements aux exceptions
 */
class Exception
{
};


#endif // NL_TYPES_H
