/** \file debug.h
 * This file contains all features that help us to debug applications
 *
 * $Id: debug.h,v 1.58 2003/10/23 16:15:34 distrib Exp $
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
#include "nel/misc/displayer.h"

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
extern CMsgBoxDisplayer *DefaultMsgBoxDisplayer;

//
// Functions
//

/// Never use this function (internal use only)
void nlFatalError (const char *format, ...);

/// Never use this function but call the nlerror macro (internal use only)
void nlError (const char *format, ...);

// internal use only
void createDebug (const char *logPath = NULL, bool logInFile = true);

// call this if you want to change the dir of the log.log file
void changeLogDirectory(const std::string &dir);

// internal breakpoint window
void enterBreakpoint (const char *message);

// if true, the assert generates an assert
// if false, the assert just displays a warning and continue
void setAssert (bool assert);


// Need a breakpoint in the assert / verify macro
extern bool DebugNeedAssert;

// Internal process, don't use it
extern bool NoAssert;

// Internal process, don't use it
extern bool GlobalAssertCall;

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
#ifdef NL_RELEASE
#	define nldebug 0&&
#else // NL_RELEASE
#	ifdef NL_OS_WINDOWS
#		define nldebug NLMISC::createDebug (), NLMISC::DebugLog->setPosition( __LINE__, __FILE__ ), NLMISC::_nldebug
#	else
#		define nldebug NLMISC::createDebug (), NLMISC::DebugLog->setPosition( __LINE__, __FILE__ ), NLMISC::DebugLog->displayNL
#	endif
#endif // NL_RELEASE

/**
 * \def nlinfo(exp)
 * Same as nldebug but it will be display in debug and in release mode.
 */
#ifdef NL_RELEASE
#	define nlinfo 0&&
#else // NL_RELEASE
#	ifdef NL_OS_WINDOWS
#		define nlinfo NLMISC::createDebug (), NLMISC::InfoLog->setPosition( __LINE__, __FILE__ ), NLMISC::_nlinfo
#	else
#		define nlinfo NLMISC::createDebug (), NLMISC::InfoLog->setPosition( __LINE__, __FILE__ ), NLMISC::InfoLog->displayNL
#	endif
#endif // NL_RELEASE

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

#ifdef NL_RELEASE
#	define nlwarning 0&&
#else // NL_RELEASE
#	ifdef NL_OS_WINDOWS
#		define nlwarning NLMISC::createDebug (), NLMISC::WarningLog->setPosition( __LINE__, __FILE__ ), NLMISC::_nlwarning
#	else
#		define nlwarning NLMISC::createDebug (), NLMISC::WarningLog->setPosition( __LINE__, __FILE__ ), NLMISC::WarningLog->displayNL
#	endif
#endif // NL_RELEASE

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
#define NLMISC_BREAKPOINT abort()
#endif

// removed because we always check assert (even in release mode) #if defined(NL_DEBUG)

#ifdef NL_RELEASE
#define nlassert(exp) \
if(false)
#define nlassertonce(exp) \
if(false)
#define nlassertex(exp, str) \
if(false)
#define nlverify(exp) \
{ exp; }
#define nlverifyonce(exp) \
{ exp; }
#define nlverifyex(exp, str) \
{ exp; }
#else // NL_RELEASE

#define nlassert(exp) \
{ \
	static bool ignoreNextTime = false; \
	if (!ignoreNextTime && !(exp)) { \
		NLMISC::DebugNeedAssert = false; \
		NLMISC::createDebug (); \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime = ignoreNextTime; \
		else if(!NLMISC::NoAssert) \
			NLMISC::DebugNeedAssert = true; \
		NLMISC::AssertLog->setPosition (__LINE__, __FILE__); \
		NLMISC::AssertLog->displayNL ("\"%s\" ", #exp); \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			ignoreNextTime = NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime; \
		if (NLMISC::DebugNeedAssert) \
			NLMISC_BREAKPOINT; \
		NLMISC::GlobalAssertCall = true; \
	} \
}/*for(;NLMISC::GlobalAssertCall;NLMISC::GlobalAssertCall=false) if (!(exp) && NLMISC::NoAssert)*/

#define nlassertonce(exp) \
{ \
	static bool ignoreNextTime = false; \
	if (!ignoreNextTime && !(exp)) { \
		ignoreNextTime = true; \
		NLMISC::DebugNeedAssert = false; \
		NLMISC::createDebug (); \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime = ignoreNextTime; \
		else if(!NLMISC::NoAssert) \
			NLMISC::DebugNeedAssert = true; \
		NLMISC::AssertLog->setPosition (__LINE__, __FILE__); \
		NLMISC::AssertLog->displayNL ("\"%s\" ", #exp); \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			ignoreNextTime = NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime; \
		if (NLMISC::DebugNeedAssert) \
			NLMISC_BREAKPOINT; \
		NLMISC::GlobalAssertCall = true; \
	} \
}/*for(;NLMISC::GlobalAssertCall;NLMISC::GlobalAssertCall=false) if (!(exp) && NLMISC::NoAssert)*/

#define nlassertex(exp, str) \
{ \
	static bool ignoreNextTime = false; \
	if (!ignoreNextTime && !(exp)) { \
		NLMISC::DebugNeedAssert = false; \
		NLMISC::createDebug (); \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime = ignoreNextTime; \
		else if(!NLMISC::NoAssert) \
			NLMISC::DebugNeedAssert = true; \
		NLMISC::AssertLog->setPosition (__LINE__, __FILE__); \
		NLMISC::AssertLog->display ("\"%s\" ", #exp); \
		NLMISC::AssertLog->displayRawNL str; \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			ignoreNextTime = NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime; \
		if (NLMISC::DebugNeedAssert) \
			NLMISC_BREAKPOINT; \
		NLMISC::GlobalAssertCall = true; \
	} \
}/*for(;NLMISC::GlobalAssertCall;NLMISC::GlobalAssertCall=false) if (!(exp) && NLMISC::NoAssert)*/

#define nlverify(exp) \
{ \
	static bool ignoreNextTime = false; \
	if (!ignoreNextTime && !(exp)) { \
		NLMISC::DebugNeedAssert = false; \
		NLMISC::createDebug (); \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime = ignoreNextTime; \
		else \
			NLMISC::DebugNeedAssert = true; \
		NLMISC::AssertLog->setPosition (__LINE__, __FILE__); \
		NLMISC::AssertLog->displayNL ("\"%s\" ", #exp); \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			ignoreNextTime = NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime; \
		if (NLMISC::DebugNeedAssert) \
			NLMISC_BREAKPOINT; \
	} \
}

#define nlverifyonce(exp) \
{ \
	static bool ignoreNextTime = false; \
	if (!ignoreNextTime && !(exp)) { \
		ignoreNextTime = true; \
		NLMISC::DebugNeedAssert = false; \
		NLMISC::createDebug (); \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime = ignoreNextTime; \
		else \
			NLMISC::DebugNeedAssert = true; \
		NLMISC::AssertLog->setPosition (__LINE__, __FILE__); \
		NLMISC::AssertLog->displayNL ("\"%s\" ", #exp); \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			ignoreNextTime = NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime; \
		if (NLMISC::DebugNeedAssert) \
			NLMISC_BREAKPOINT; \
	} \
}

#define nlverifyex(exp, str) \
{ \
	static bool ignoreNextTime = false; \
	if (!ignoreNextTime && !(exp)) { \
		NLMISC::DebugNeedAssert = false; \
		NLMISC::createDebug (); \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime = ignoreNextTime; \
		else \
			NLMISC::DebugNeedAssert = true; \
		NLMISC::AssertLog->setPosition (__LINE__, __FILE__); \
		NLMISC::AssertLog->display ("\"%s\" ", #exp); \
		NLMISC::AssertLog->displayRawNL str; \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			ignoreNextTime = NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime; \
		if (NLMISC::DebugNeedAssert) \
			NLMISC_BREAKPOINT; \
	} \
}
#endif // NL_RELEASE


#define nlstop \
{ \
	static bool ignoreNextTime = false; \
	if (!ignoreNextTime) { \
		NLMISC::DebugNeedAssert = false; \
		NLMISC::createDebug (); \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime = ignoreNextTime; \
		else if(!NLMISC::NoAssert) \
			NLMISC::DebugNeedAssert = true; \
		NLMISC::AssertLog->setPosition (__LINE__, __FILE__); \
		NLMISC::AssertLog->displayNL ("STOP"); \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			ignoreNextTime = NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime; \
		if (NLMISC::DebugNeedAssert) \
			NLMISC_BREAKPOINT; \
		NLMISC::GlobalAssertCall = true; \
	} \
}/*for(;NLMISC::GlobalAssertCall;NLMISC::GlobalAssertCall=false) if (NLMISC::NoAssert)*/

#define nlstoponce \
{ \
	static bool ignoreNextTime = false; \
	if (!ignoreNextTime) { \
		ignoreNextTime = true; \
		NLMISC::DebugNeedAssert = false; \
		NLMISC::createDebug (); \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime = ignoreNextTime; \
		else if(!NLMISC::NoAssert) \
			NLMISC::DebugNeedAssert = true; \
		NLMISC::AssertLog->setPosition (__LINE__, __FILE__); \
		NLMISC::AssertLog->displayNL ("STOP"); \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			ignoreNextTime = NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime; \
		if (NLMISC::DebugNeedAssert) \
			NLMISC_BREAKPOINT; \
		NLMISC::GlobalAssertCall = true; \
	} \
}/*for(;NLMISC::GlobalAssertCall;NLMISC::GlobalAssertCall=false) if (NLMISC::NoAssert)*/


#define nlstopex(str) \
{ \
	static bool ignoreNextTime = false; \
	if (!ignoreNextTime) { \
		NLMISC::DebugNeedAssert = false; \
		NLMISC::createDebug (); \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime = ignoreNextTime; \
		else if(!NLMISC::NoAssert) \
			NLMISC::DebugNeedAssert = true; \
		NLMISC::AssertLog->setPosition (__LINE__, __FILE__); \
		NLMISC::AssertLog->display ("STOP "); \
		NLMISC::AssertLog->displayRawNL str; \
		if (NLMISC::DefaultMsgBoxDisplayer) \
			ignoreNextTime = NLMISC::DefaultMsgBoxDisplayer->IgnoreNextTime; \
		if (NLMISC::DebugNeedAssert) \
			NLMISC_BREAKPOINT; \
		NLMISC::GlobalAssertCall = true; \
	} \
}/*for(;NLMISC::GlobalAssertCall;NLMISC::GlobalAssertCall=false) if (NLMISC::NoAssert)*/


struct EFatalError : public Exception
{
	EFatalError() : Exception( "nlerror() called" ) {}
};

class ETrapDebug : public Exception
{
};

// undef default assert to force people to use nlassert() instead of assert()
#ifdef assert
#undef assert
#endif
#define assert(a) you_must_not_use_assert___use_nl_assert___read_debug_h_file


/// Get the call stack and set it with result
void getCallStackAndLog (std::string &result, sint skipNFirst = 0);

/**
 * safe_cast<>: this is a function which nlassert() a dynamic_cast in Debug, and just do a static_cast in release.
 * So slow check is made in debug, but only fast cast is made in release.
 */
template<class T, class U>	inline T	safe_cast(U o)
{
	// NB: must check debug because assert may still be here in release
#ifdef	NL_DEBUG
	nlassert(dynamic_cast<T >(o));
#endif
	return static_cast<T >(o);
}

/**
 * type_cast<>: this is a function which nlassert() a dynamic_cast in Debug, and just do a static_cast in release.
 * So slow check is made in debug, but only fast cast is made in release.
 * Differs from safe_cast by allowinf NULL objets.
 */
template<class T, class U>	inline T	type_cast(U o)
{
	// NB: must check debug because assert may still be here in release
#ifdef	NL_DEBUG
	if (o)
		nlassert(dynamic_cast<T >(o));
#endif
	//	optimization made to check pointeur validity before adresse translation. (hope it works on linux).
	if ((long)(static_cast<T>((U)0x0400))==(long)((U)0x0400))
	{
		return static_cast<T >(o);
	}
	else
	{
		return	(o==NULL)?NULL:static_cast<T >(o);
	}

}

/** Compile time assertion
  */
#define nlctassert(cond) sizeof(uint[(cond) ? 1 : 0]);


// Beep (Windows only, no effect elsewhere)
void beep( uint freq, uint duration );




#ifdef NL_OS_WINDOWS

//
// These functions are needed by template system to failed the compilation if you pass bad type on nlinfo...
//

inline void _check(int a) { }
inline void _check(unsigned int a) { }
inline void _check(char a) { }
inline void _check(unsigned char a) { }
inline void _check(long a) { }
inline void _check(unsigned long a) { }
inline void _check(uint8 a) { }
inline void _check(sint8 a) { }
inline void _check(uint16 a) { }
inline void _check(sint16 a) { }
inline void _check(uint32 a) { }
inline void _check(sint32 a) { }
inline void _check(uint64 a) { }
inline void _check(sint64 a) { }
inline void _check(float a) { }
inline void _check(double a) { }
inline void _check(const char *a) { }
inline void _check(const void *a) { }

inline void _nldebug(const char *fmt) { NLMISC::DebugLog->displayNL(fmt); }
template<class A> void _nldebug(const char *fmt, A a) { _check(a); NLMISC::DebugLog->displayNL(fmt, a); }
template<class A, class B> void _nldebug(const char *fmt, A a, B b) { _check(a); _check(b); NLMISC::DebugLog->displayNL(fmt, a, b); }
template<class A, class B, class C> void _nldebug(const char *fmt, A a, B b, C c) { _check(a); _check(b); _check(c); NLMISC::DebugLog->displayNL(fmt, a, b, c); }
template<class A, class B, class C, class D> void _nldebug(const char *fmt, A a, B b, C c, D d) { _check(a); _check(b); _check(c); _check(d); NLMISC::DebugLog->displayNL(fmt, a, b, c, d); }
template<class A, class B, class C, class D, class E> void _nldebug(const char *fmt, A a, B b, C c, D d, E e) { _check(a); _check(b); _check(c); _check(d); _check(e); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e); }
template<class A, class B, class C, class D, class E, class F> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f); }
template<class A, class B, class C, class D, class E, class F, class G> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g); }
template<class A, class B, class C, class D, class E, class F, class G, class H> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W, class X> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w, X x) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); _check(x); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W, class X, class Y> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w, X x, Y y) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); _check(x); _check(y); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W, class X, class Y, class Z> void _nldebug(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w, X x, Y y, Z z) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); _check(x); _check(y); _check(z); NLMISC::DebugLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z); }

inline void _nlinfo(const char *fmt) { NLMISC::InfoLog->displayNL(fmt); }
template<class A> void _nlinfo(const char *fmt, A a) { _check(a); NLMISC::InfoLog->displayNL(fmt, a); }
template<class A, class B> void _nlinfo(const char *fmt, A a, B b) { _check(a); _check(b); NLMISC::InfoLog->displayNL(fmt, a, b); }
template<class A, class B, class C> void _nlinfo(const char *fmt, A a, B b, C c) { _check(a); _check(b); _check(c); NLMISC::InfoLog->displayNL(fmt, a, b, c); }
template<class A, class B, class C, class D> void _nlinfo(const char *fmt, A a, B b, C c, D d) { _check(a); _check(b); _check(c); _check(d); NLMISC::InfoLog->displayNL(fmt, a, b, c, d); }
template<class A, class B, class C, class D, class E> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e) { _check(a); _check(b); _check(c); _check(d); _check(e); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e); }
template<class A, class B, class C, class D, class E, class F> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f); }
template<class A, class B, class C, class D, class E, class F, class G> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g); }
template<class A, class B, class C, class D, class E, class F, class G, class H> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W, class X> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w, X x) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); _check(x); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W, class X, class Y> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w, X x, Y y) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); _check(x); _check(y); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W, class X, class Y, class Z> void _nlinfo(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w, X x, Y y, Z z) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); _check(x); _check(y); _check(z); NLMISC::InfoLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z); }

inline void _nlwarning(const char *fmt) { NLMISC::WarningLog->displayNL(fmt); }
template<class A> void _nlwarning(const char *fmt, A a) { _check(a); NLMISC::WarningLog->displayNL(fmt, a); }
template<class A, class B> void _nlwarning(const char *fmt, A a, B b) { _check(a); _check(b); NLMISC::WarningLog->displayNL(fmt, a, b); }
template<class A, class B, class C> void _nlwarning(const char *fmt, A a, B b, C c) { _check(a); _check(b); _check(c); NLMISC::WarningLog->displayNL(fmt, a, b, c); }
template<class A, class B, class C, class D> void _nlwarning(const char *fmt, A a, B b, C c, D d) { _check(a); _check(b); _check(c); _check(d); NLMISC::WarningLog->displayNL(fmt, a, b, c, d); }
template<class A, class B, class C, class D, class E> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e) { _check(a); _check(b); _check(c); _check(d); _check(e); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e); }
template<class A, class B, class C, class D, class E, class F> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f); }
template<class A, class B, class C, class D, class E, class F, class G> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g); }
template<class A, class B, class C, class D, class E, class F, class G, class H> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W, class X> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w, X x) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); _check(x); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W, class X, class Y> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w, X x, Y y) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); _check(x); _check(y); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y); }
template<class A, class B, class C, class D, class E, class F, class G, class H, class I, class J, class K, class L, class M, class N, class O, class P, class Q, class R, class S, class T, class U, class V, class W, class X, class Y, class Z> void _nlwarning(const char *fmt, A a, B b, C c, D d, E e, F f, G g, H h, I i, J j, K k, L l, M m, N n, O o, P p, Q q, R r, S s, T t, U u, V v, W w, X x, Y y, Z z) { _check(a); _check(b); _check(c); _check(d); _check(e); _check(f); _check(g); _check(h); _check(i); _check(j); _check(k); _check(l); _check(m); _check(n); _check(o); _check(p); _check(q); _check(r); _check(s); _check(t); _check(u); _check(v); _check(w); _check(x); _check(y); _check(z); NLMISC::WarningLog->displayNL(fmt, a, b, c, d, e, f, g, h, i, j, k, l, m, n, o, p, q, r, s, t, u, v, w, x, y, z); }

#endif // NL_OS_WINDOWS



} // NLMISC


#endif // NL_DEBUG_H

/* End of debug.h */
