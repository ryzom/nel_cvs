/** \file hierarchical_timer.h
 * Hierarchical timer
 *
 * $Id: hierarchical_timer.h,v 1.1 2002/03/13 13:52:46 cado Exp $
 */

/* Copyright, 2000, 2001 Nevrax Ltd.
 *
 * This file is part of NEVRAX NeL Network Services.
 * NEVRAX NeL Network Services is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NeL Network Services is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NeL Network Services; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_HIERARCHICAL_TIMER_H
#define NL_HIERARCHICAL_TIMER_H

#include <string>
#include <vector>

#include "nel/misc/types_nl.h"
#include "nel/misc/time_nl.h"
#include "nel/misc/debug.h"

#define ALLOW_TIMING_MEASURES


// Several macros to use

#define H_TIME(name, inst) \
{ \
	static CHTimer	nl_h_timer(name); \
	nl_h_timer.before(); \
	inst \
	nl_h_timer.after(); \
}



#define H_BEFORE(__name)	static CHTimer	__name##_timer(#__name); __name##_timer.before();
#define H_AFTER(__name)		__name##_timer.after();

//#define H_AUTO(__name)		static CHTimer	__name##_timer(#_name); CAutoTimer	__name##_auto(__name##_timer);



#ifdef ALLOW_TIMING_MEASURES

/**
 * Hierarchical timing system. Allows to accurately measure performance of routines, and displays results hierarchically.
 * To time a piece of code, just declare a static CHTimer object and encapsulate code between calls to before() and after() methods.
 * ex:
 *\code
 void myFunction()
 {
	static CHTimer	myTimer("myFunction");
	myTimer.before();
	// some code here
	myTimer.after();
 }
 *\endcode
 * Don't forget to call after() to avoid timing wrongness or assertion crashes !
 *
 * \warning Supports only single-threaded applications.
 * \warning Supports only Intel processors.
 *
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CHTimer
{
private:
	const char						*_Name;
	CHTimer							*_Parent;
	std::vector<CHTimer*>			_Children;

	uint32							_StartTick;
	sint64							_TotalTicks;
	sint64							_AdjustedTicks;
	uint32							_NumTests;
	uint32							_MinTicks;
	uint32							_MaxTicks;

	static std::vector<CHTimer*>	_Stack;
	static std::vector<CHTimer*>	_Roots;
	static double					_TicksPerTest;
	static double					_MsPerTick;

public:
	/// Constructor.
	CHTimer(const char *name) : _Name(name), _Parent(NULL), _TotalTicks(0), _NumTests(0), _MinTicks(0xffffffff), _MaxTicks(0) {}

	/// Starts a measuring session
	void	before()
	{
		if (_Parent == NULL)
		{
			if (_Stack.empty())
			{
				_Roots.push_back(this);
				_Parent = this;
			}
			else
			{
				_Parent = _Stack.back();
				_Parent->_Children.push_back(this);
			}
		}

		_Stack.push_back(this);

#ifdef NL_OS_WINDOWS
		uint32	tick;
		__asm	rdtsc
		__asm	mov		tick, eax
		_StartTick = tick;
#else
		unsigned long long int x;
		__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
		_StartTick = (uint32)x;
#endif
	}

	/// Stops a measuring session
	void	after()
	{
		uint32	dt;
#ifdef NL_OS_WINDOWS
		uint32	tick;
		__asm	rdtsc
		__asm	mov		tick, eax
		dt = ((tick-_StartTick) >> 1);
#else
		unsigned long long int x;
		__asm__ volatile (".byte 0x0f, 0x31" : "=A" (x));
		dt = (((uint32)x-_StartTick) >> 1);
#endif
		_TotalTicks += dt;
		if (dt < _MinTicks)	_MinTicks = dt;
		if (dt > _MaxTicks)	_MaxTicks = dt;
		nlassertex(!_Stack.empty(), ("in %s", _Name));
		nlassertex(_Stack.back() == this, ("in %s, _StackTop at %s", _Name, _Stack.back()->_Name));
		++_NumTests;
		_Stack.pop_back();
	}

	//

	/// Call bench() before any test (sets own timer loop perf up)
	static void		bench();

	/// Adjust values recorded according to the bench previously performed
	static void		adjust();

	/// Display results hierarchically
	static void		display();

	/// Clears stats, and reinits all timer structure
	static void		clear();

private:

	void			displayNode(uint level, CHTimer *root);

	void			clearNode()
	{
		_Parent = NULL;
		_TotalTicks = 0;
		_AdjustedTicks = 0;
		_NumTests = 0;
		_MinTicks = 0xffffffff;
		_MaxTicks = 0;

		uint	i;
		for (i=0; i<_Children.size(); ++i)
			_Children[i]->clearNode();

		_Children.clear();
	}

	uint32			adjustNode();
};

/**
 * An automatic measuring timer. Encapsulates calls to CHTimer, and avoids missuses of before() and after().
 * ex:
 *\code
 void myFunction()
 {
	static CHTimer	myTimer("myFunction");
	CAutoTimer		myAuto(myTimer);
	// some code here
 }
 *\endcode
 * Don't forget to call after() to avoid timing wrongness or assertion crashes !
 * \author Benjamin Legros
 * \author Nevrax France
 * \date 2001
 */
class CAutoTimer
{
private:
	CHTimer		*_HTimer;
public:
	CAutoTimer(CHTimer *timer) : _HTimer(timer) { _HTimer->before(); }
	~CAutoTimer() { _HTimer->after(); }
};

#else // ALLOW_TIMING_MEASURES

class CHTimer
{
public:
	CHTimer(const char *name) {}

	void	before() {}

	void	after()	{}

	static void		bench() {}

	static void		display() {}

	static void		clear() {}

	static void		adjust() {}

private:

	void			displayNode(uint level, CHTimer *root) {}

	void			clearNode() {}

	uint32			adjustNode() {}
};

class CAutoTimer
{
public:
	CAutoTimer(CHTimer *timer) {}
	~CAutoTimer() {}
};

#endif // ALLOW_TIMING_MEASURES

#endif // NL_HIERARCHICAL_TIMER_H

/* End of hierarchical_timer.h */
