/** \file debug.h
 * This file contains all features that help us to debug applications
 *
 * $Id: debug.h,v 1.38 2002/03/14 13:49:36 lecroart Exp $
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

#ifndef NL_DEBUG_H
#define NL_DEBUG_H

#include <stdio.h>

#include "nel/misc/common.h"
#include "nel/misc/log.h"
#include "nel/misc/mutex.h"
#include "nel/misc/mem_displayer.h"

namespace NLMISC
{

//
// Externals
//

extern CLog *ErrorLog;
extern CLog *WarningLog;
extern CLog *InfoLog;
extern CLog *DebugLog;
extern CLog *AssertLog;

extern CMemDisplayer *DefaultMemDisplayer;

//
// Functions
//

/// Never use this function (internal use only)
void nlFatalError (const char *format, ...);

/// Never use this function but call the nlerror macro (internal use only)
void nlError (const char *format, ...);

// internal use only
void createDebug ();

// Macros


/**
 * \def nldebug(exp)
 * Log a debug string. You don't have to put the final new line. It will be automatically append at the end of the string.
 *
 * Example:
 *\code
	void function(sint type)
	{
		// display the type value.
		nldebug("type is %d", type);
	}
 *\endcode
 */
#define nldebug NLMISC::createDebug (), NLMISC::DebugLog->setPosition( __LINE__, __FILE__ ), NLMISC::DebugLog->displayNL


/**
 * \def nlinfo(exp)
 * Same as nldebug but it will be display in debug and in release mode.
 */
#define nlinfo NLMISC::createDebug (), NLMISC::InfoLog->setPosition( __LINE__, __FILE__ ), NLMISC::InfoLog->displayNL


/**
 * \def nlwarning(exp)
 * Same as nlinfo but you have to call this macro when something goes wrong but it's not a fatal error, the program could continue.
 *
 * Example:
 *\code
	void function(char *str)
	{
		// display the type value.
		if (str==NULL)
		{
			nlwarning("in function(), str should not be NULL, assume it's an empty string");
			str="";
		}
	}
 *\endcode
 */
#define nlwarning NLMISC::createDebug (), NLMISC::WarningLog->setPosition( __LINE__, __FILE__ ), NLMISC::WarningLog->displayNL


/**
 * \def nlerror(exp)
 * Same as nlinfo but you have to call it when you have a fatal error, this macro display the text and \b exit the application
 * automatically. nlerror must be in a try/catch because it generates an EFatalError exception to exit the application.
 *
 *\code
	void function(char *filename)
	{
		FILE *fp = fopen (filename, "r");
		if (fp==NULL)
		{
			nlerror("file not found");
		}
	}
 *\endcode
 */
#define nlerror NLMISC::createDebug (), NLMISC::ErrorLog->setPosition( __LINE__, __FILE__ ), NLMISC::nlFatalError


/**
 * \def nlerrornoex(exp)
 * Same as nlerror but it doesn't generate any exceptions. It's used only in very specific case, for example, when you
 * call a nlerror in a catch block (look the service.cpp)
 */
#define nlerrornoex NLMISC::createDebug (), NLMISC::ErrorLog->setPosition( __LINE__, __FILE__ ), NLMISC::nlError


/**
 * \def nlassert(exp)
 * Try the assertion of \c exp. In release mode, nlassert do \b *nothing*. In debug mode, If \c exp is true, nothing happen,
 * otherwise, the program stop on the assertion line.
 *
 * Example:
 *\code
	void function(char *string)
	{
		// the string must not be NULL.
		nlassert(string!=NULL);
	}
 *\endcode
 */

/**
 * \def nlassertonce(exp)
 * Same behaviour as nlassert but the assertion will be test on time only. It's useful when you are under an inner loop and
 * you don't want to flood log file for example.
 *
 * Example:
 *\code
	for (int i = 0; i < Vect.size(); i++)
	{
		// all string must not be NULL, but if it's happen, log only the first time.
		nlassertonce(Vect[i].string!=NULL);
	}
 *\endcode
 */

/**
 * \def nlassertex(exp,str)
 * Same behaviour as nlassert but add a user defined \c str variables args string that will be display with the assert message.
 * Very useful when you can't debug directly on your computer.
 *
 * Example:
 *\code
	void function(sint type)
	{
		// the \c type must be between 0 and 15.
		nlassertex(type>=0&&type<=16, ("type was %d", type));
		// it'll display something like "assertion failed line 10 of test.cpp: type>=&&type<=16, type was 423555463"
	}
 *\endcode
 */

/**
 * \def nlverify(exp)
 * Same behaviour as nlassert but the \c exp will be executed in release mode too (but not tested).
 *
 * Example:
 *\code
	// Load a file and assert if the load failed. This example will work \b only in debug mode because in release mode,
	// nlassert do nothing, the the load function will not be called...
	nlassert(load("test.tga"));

	// If you want to do that anyway, you could call nlverify. In release mode, the assertion will not be tested but
	// the \c load function will be called.
	nlverify(load("test.tga"));

	// You can also do this:
	bool res = load ("test.tga"));
	assert(res);
 
 *\endcode
 */

/**
 * \def nlverifyonce(exp)
 * Same behaviour as nlassertonce but it will execute \c exp in debug and release mode.
 */

/**
 * \def nlverifyex(exp,str)
 * Same behaviour as nlassertex but it will execute \c exp in debug and release mode.
 */

/**
 * \def nlstop
 * It stop the application at this point. It's exactly the same thing as "nlassert(false)".
 * Example:
 *\code
	switch(type)
	{
	case 1: ... break;
	case 2: ... break;
	default: nlstop;	// it should never happen...
	}
 *\endcode
 */

/**
 * \def nlstoponce
 * Same as nlassertonce(false);
 */

/**
 * \def nlstopex(exp)
 * Same as nlassertex(false,exp);
 */

// removed because we always check assert (even in release mode) #if defined (NL_OS_WINDOWS) && defined (NL_DEBUG)
#if defined (NL_OS_WINDOWS)
#define NLMISC_BREAKPOINT _asm { int 3 }
#else
#define NLMISC_BREAKPOINT
#endif

// removed because we always check assert (even in release mode) #if defined(NL_DEBUG)

#define nlassert(exp) \
{ \
	if (!(exp)) { \
		NLMISC::createDebug (); \
		NLMISC::AssertLog->setPosition (__LINE__, __FILE__); \
		NLMISC::AssertLog->displayNL ("\"%s\" ", #exp); \
		NLMISC::DefaultMemDisplayer->write (); \
		NLMISC_BREAKPOINT \
	} \
}

#define nlassertonce(exp) \
{ \
	static bool ignoreAlways = false; \
	if (!ignoreAlways && !(exp)) { \
		ignoreAlways=true; \
		NLMISC::createDebug (); \
		NLMISC::AssertLog->setPosition( __LINE__, __FILE__ ); \
		NLMISC::AssertLog->displayNL ("\"%s\" ", #exp); \
		NLMISC::DefaultMemDisplayer->write (); \
		NLMISC_BREAKPOINT \
	} \
}

#define nlassertex(exp, str) \
{ \
	if (!(exp)) \
	{ \
		NLMISC::createDebug (); \
		NLMISC::AssertLog->setPosition( __LINE__, __FILE__ ); \
		NLMISC::AssertLog->display ("\"%s\" ", #exp); \
		NLMISC::AssertLog->displayRawNL str; \
		NLMISC::DefaultMemDisplayer->write (); \
		NLMISC_BREAKPOINT \
	} \
}

#define nlverify(exp) \
{ \
	if (!(exp)) { \
		NLMISC::createDebug (); \
		NLMISC::AssertLog->setPosition (__LINE__, __FILE__); \
		NLMISC::AssertLog->displayNL ("\"%s\" ", #exp); \
		NLMISC::DefaultMemDisplayer->write (); \
		NLMISC_BREAKPOINT \
	} \
}

#define nlverifyonce(exp) \
{ \
	static bool ignoreAlways = false; \
	if (!ignoreAlways && !(exp)) { \
		ignoreAlways=true; \
		NLMISC::createDebug (); \
		NLMISC::AssertLog->setPosition ( __LINE__, __FILE__ ); \
		NLMISC::AssertLog->displayNL ("\"%s\" ", #exp); \
		NLMISC::DefaultMemDisplayer->write (); \
		NLMISC_BREAKPOINT \
	} \
}

#define nlverifyex(exp, str) \
{ \
	if (!(exp)) \
	{ \
		NLMISC::createDebug (); \
		NLMISC::AssertLog->setPosition ( __LINE__, __FILE__ ); \
		NLMISC::AssertLog->display ("\"%s\" ", #exp); \
		NLMISC::AssertLog->displayRawNL str; \
		NLMISC::DefaultMemDisplayer->write (); \
		NLMISC_BREAKPOINT \
	} \
}

#define nlstop \
{ \
	NLMISC::createDebug (); \
	NLMISC::AssertLog->setPosition (__LINE__, __FILE__); \
	NLMISC::AssertLog->displayNL ("STOP "); \
	NLMISC::DefaultMemDisplayer->write (); \
	NLMISC_BREAKPOINT \
}

#define nlstoponce \
{ \
	static bool ignoreAlways = false; \
	if (!ignoreAlways) { \
		ignoreAlways=true; \
		NLMISC::createDebug (); \
		NLMISC::AssertLog->setPosition ( __LINE__, __FILE__ ); \
		NLMISC::AssertLog->displayNL ("STOP "); \
		NLMISC::DefaultMemDisplayer->write (); \
		NLMISC_BREAKPOINT \
	} \
}

#define nlstopex(str) \
{ \
	NLMISC::createDebug (); \
	NLMISC::AssertLog->setPosition ( __LINE__, __FILE__ ); \
	NLMISC::AssertLog->display ("STOP "); \
	NLMISC::AssertLog->displayRawNL str; \
	NLMISC::DefaultMemDisplayer->write (); \
	NLMISC_BREAKPOINT \
}

/* removed because we always check assert (even in release mode) 
#else // NL_DEBUG

#define nlassert(exp) \
NULL
#define nlassertonce(exp) \
NULL
#define nlassertex(exp, str) \
NULL
#define nlverify(exp) \
{ exp; }
#define nlverifyonce(exp) \
{ exp; }
#define nlverifyex(exp, str) \
{ exp; }
#define nlstop \
NULL
#define nlstoponce \
NULL
#define nlstopex(str) \
NULL

#endif // NL_DEBUG
*/

struct EFatalError : public Exception
{
	EFatalError() : Exception( "nlerror() called" ) {}
};


// undef default assert to force people to use nlassert() instead of assert()
#ifdef assert
#undef assert
#endif
#define assert(a) you_must_not_use_assert___use_nl_assert___read_debug_h_file



/**
 * safe_cast<>: this is a function which nlassert() a dynamic_cast in Debug, and just do a static_cast in release.
 * So slow check is made in debug, but only fast cast is made in release.
 */
template<class T, class U>	inline T	safe_cast(U o)
{
	nlassert(dynamic_cast<T>(o));
	return static_cast<T>(o);
}



} // NLMISC


#endif // NL_DEBUG_H

/* End of debug.h */
