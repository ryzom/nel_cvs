/** \file hierarchical_timer.h
 * Hierarchical timer
 *
 * $Id: hierarchical_timer.h,v 1.16 2002/06/10 16:51:09 berenguier Exp $
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

#include <algorithm>

#define ALLOW_TIMING_MEASURES


#ifdef ALLOW_TIMING_MEASURES
	// Several macros to use
#	define H_TIME(name, inst) \
	{ \
		static NLMISC::CHTimer	nl_h_timer(name); \
		nl_h_timer.before(); \
		inst \
		nl_h_timer.after(); \
	}
	//
#	define H_BEFORE(__name)	static NLMISC::CHTimer	__name##_timer(#__name); __name##_timer.before();
#	define H_AFTER(__name)		__name##_timer.after();
	//
#	define H_AUTO(__name)		static NLMISC::CHTimer	__name##_timer(#__name); NLMISC::CAutoTimer	__name##_auto(&__name##_timer);
	// display the timer info after each loop call
#	define H_AUTO_INST(__name)	static NLMISC::CHTimer	__name##_timer(#__name); NLMISC::CAutoTimer	__name##_auto(&__name##_timer, true);

/** H_AUTO splitted in 2. The declaration of the static timer, and a CAutoTimer instance.
 *	Usefull to group same timer bench in different functions for example
 */
#	define H_AUTO_DECL(__name)		static NLMISC::CHTimer	__name##_timer(#__name); 
#	define H_AUTO_USE(__name)		NLMISC::CAutoTimer	__name##_auto(&__name##_timer);

#else
	// void macros
#	define H_TIME(name, inst)	
#	define H_BEFORE(__name)
#	define H_AFTER(__name)	
#	define H_AUTO(__name)	
#	define H_AUTO_INST(__name)
#	define H_AUTO_DECL(__name)
#	define H_AUTO_USE(__name)
#endif


namespace NLMISC
{


#ifdef NL_OS_WINDOWS
// Vicual C++ warning : ebp maybe modified
#	pragma warning(disable:4731)
#endif


/** Read the time stamp counter. Supports only intel architectures for now  
  */ 
#ifdef NL_CPU_INTEL

inline uint64 rdtsc()
{
	uint64 ticks;
#	ifndef NL_OS_WINDOWS		
		__asm__ volatile(".byte 0x0f, 0x31" : "=a" (ticks.low), "=d" (ticks.high));				
#	else 		
		__asm	rdtsc
		__asm	mov		DWORD PTR [ticks], eax
		__asm	mov		DWORD PTR [ticks + 4], edx		
#	endif
	return ticks;	
}

#endif	


/**  A simple clock to measure ticks.
  *  \warning On intel platform, processor cycles are counted, on other platforms, CTime::getPerformanceTime is used instead.
  *  
  * \sa CStopWatch
  * \author Nicolas Vizerie
  * \author Nevrax France
  * \date 2002
  */
class CSimpleClock
{
public:	
	CSimpleClock() : _NumTicks(0)
	{
#		ifdef NL_DEBUG
			_Started = false;
#		endif
	}
	// start measure
	void start()
	{
#		ifdef  NL_DEBUG
			nlassert(!_Started);
			_Started = true;
#		endif
#		ifdef NL_CPU_INTEL
		_StartTick = rdtsc();
#		else
		_StartTick = CTime::getPerformanceTime();
#		endif
	}
	// end measure
	void stop()
	{
#		ifdef  NL_DEBUG
			nlassert(_Started);
			_Started = false;
#		endif
#		ifdef NL_CPU_INTEL
		_NumTicks = rdtsc() - _StartTick;
#		else
		_NumTicks = CTime::getPerformanceTime() - _StartTick;
#		endif
		
	}	
	// get measure
	uint64	getNumTicks() const
	{
		#ifdef NL_DEBUG
			nlassert(!_Started);
		#endif
		nlassert(_NumTicks != 0);
		return _NumTicks;
	}	
	// This compute the duration of start and stop (in cycles).
	static void init();
	/** Get the number of ticks needed to perform start().
	  * Should have called init() before calling this.
	  */
	static uint64 getStartStopNumTicks() 
	{ 		
		return _StartStopNumTicks; 
	}	
private:
	uint64  _StartTick;
	uint64	_NumTicks;
#	ifdef  NL_DEBUG
		bool	_Started;
#	endif
	static bool		_InitDone;
	static uint64	_StartStopNumTicks;	
};





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
 * \author Nicolas Vizerie
 * \author Nevrax France
 * \date 2001, 2002
 */
class CHTimer
{
public:
	// this enum is used to sort displayed results
	enum TSortCriterion { NoSort, 
						  TotalTime,
						  TotalTimeWithoutSons,
						  MeanTime,
						  NumVisits,
						  MaxTime,
						  MinTime,
						  SortCriterionsLast
						};
public:
	/// ctor
	CHTimer(const char *name, bool isRoot = false) : _Name(name), _IsRoot(isRoot) {}
	/// Starts a measuring session
	void		before();
	// Ends a measuring session
	void		after(bool displayAfter = false);		
	// Get this node name
	const char	   *getName() const { return _Name; }
	/** Starts a bench session
	  * \param wantStandardDeviation When true, benchs will report the standard deviation of values. This require more memory, howeve, because each samples must be kept.	  
	  */
	static void		startBench(bool wantStandardDeviation = false);
	/** For backward compatibility
	  */ 
	static void		bench() { startBench(); }	
	/** For backward compatibility
	  */ 
	static void		adjust() {}
	/// Ends a bench session
	static void		endBench();
	/** Display results
	  * \param displayEx true to display more detailed infos
	  */
	static void		display(CLog *log= InfoLog, TSortCriterion criterion = TotalTime, bool displayInline = true, bool displayEx = true);
	/** Display results by execution paths	
	  * \param displayInline true to display each result on a single line.
	  * \param alignPaths    true to display all execution paths aligned.
	  * \param displayEx	 true to display more detailed infos.
	  */
	static void		displayByExecutionPath(CLog *log= InfoLog, TSortCriterion criterion = TotalTime, bool displayInline = true, bool alignPaths = true, bool displayEx = true);

	/** Hierarchical display, no sorting is done
	  * \param displayEx	 true to display more detailed infos.
	  * \param labelNumChar  
	  */
	static void		displayHierarchical(CLog *log= InfoLog, bool displayEx = true, uint labelNumChar = 32, uint indentationStep = 2);

	/** Hierarchical display, no sorting is done
	  * \param displayEx	 true to display more detailed infos.
	  * \param labelNumChar  
	  */
	static void		displayHierarchicalByExecutionPath(CLog *log= InfoLog, bool displayEx = true, uint labelNumChar = 32, uint indentationStep = 2);

	/** Hierarchical display, sorting is done in branchs
	  * \param displayEx	 true to display more detailed infos.
	  * \param labelNumChar  
	  */
	static void		displayHierarchicalByExecutionPathSorted(CLog *log= InfoLog, TSortCriterion criterion = TotalTime, bool displayEx = true, uint labelNumChar = 32, uint indentationStep = 2);

	/// Clears stats, and reinits all timer structure
	static void		clear();		

	/** Gives an evalutation of the processor frequency, in hertz
	  * \warning Supports only intel architectures for now. 
	  */
#ifdef NL_CPU_INTEL
	static uint64   getProcessorFrequency();
#endif
//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	struct CNode;
	typedef std::vector<CNode *>   TNodeVect;
	typedef std::vector<CHTimer *> TTimerVect;
	//
	/// a node in an execution path
	struct CNode
	{		
		typedef std::vector<double> TTimeVect;
		//
		CNode					*Parent;
		TNodeVect				Sons;
		CHTimer					*Owner;	   // the hierarchical timer this node is associated with		
		uint64					TotalTime; // the total time spent in that node, including sons		
		uint64					LastSonsTotalTime; 
		uint64					SonsTotalTime; // maybe different from LastSonsTotalTime while benching the sons and if the display is called in a benched node
		TTimeVect				Measures;  // All time measures. Used only when standard deviation is wanted
		uint64					MinTime;   // the minimum time spent in that node
		uint64					MaxTime;   // the maximum time spent in that node
		uint64					NumVisits; // the number of time the execution has gone through this node		
		//
		uint64					SonsPreambule; // preambule time for the sons		
		CSimpleClock			Clock;         // a clock to do the measures at this node
		// ctor 
		CNode(CHTimer	*owner = NULL, CNode	*parent = NULL) : Owner(owner), Parent(parent)
		{
			reset();
		}
		// dtor
		~CNode();
		// Get the number of nodes in the tree starting at this node
		uint  getNumNodes() const;
		// release the sons, should not be benching when calling this
		void	releaseSons();
		// reset this node measures
		void	reset()
		{
			SonsTotalTime		 = 0;
			TotalTime			 = 0;
			MaxTime				 = 0;
			MinTime				 = (uint64) -1;
			NumVisits			 = 0;
			SonsPreambule	     = 0;			
			LastSonsTotalTime    = 0;
			NLMISC::contReset(Measures);
		}
		// Display this node path
		void	displayPath(CLog *log) const;
		// Get this node path
		void    getPath(std::string &dest) const;		
	};

	/** Some statistics
	  * They can be build from a set of nodes
	  */
	struct CStats
	{	
		double  TimeStandardDeviation;
		double	TotalTime;		
		double	TotalTimeWithoutSons;
		double	MeanTime;
		uint64	NumVisits;
		double	MinTime;
		double	MaxTime;		
		
		// build stats from a vector of nodes
		void buildFromNodes(CNode **firstNode, uint numNodes, double msPerTick);

		// display stats
		void display(CLog *log, bool displayEx = false, bool wantStandardDeviation = false);

		/** Get a string for stats (all stats on the same line)
		  * \param statEx display extended stats
		  */
		void getStats(std::string &dest, bool statEx, bool wantStandardDeviation = false);
	};
	// Stats and the associated timer
	struct CTimerStat : public CStats
	{
		CHTimer *Timer;			
	};
	// Stats and the associated node
	struct CNodeStat : public CStats
	{
		CNode *Node;
	};	


	/** A statistics sorter, based on some criterion.
	  * It works on pointers on CStats objects
	  */
	struct CStatSorter
	{
		CStatSorter(TSortCriterion criterion = TotalTime) : Criterion(criterion)
		{}
		TSortCriterion Criterion;
		// Less operator
		bool operator()(const CStats *lhs, const CStats *rhs);
	};


	/** For Hierarchical + sorted display. displayHierarchicalByExecutionPath()
	 *
	 */
	struct	CExamStackEntry
	{
		// The node.
		CNode				*Node;
		// The current child to process.
		uint				CurrentChild;
		// The childs, sorted by specific criterion.
		std::vector<CNode*>	Children;

		explicit	CExamStackEntry(CNode *node)
		{
			Node= node;
			CurrentChild= 0;
		}
	};


private:
	// walk the tree to current execution node, creating it if necessary
	void	walkTreeToCurrent();
private:
	// node name
	const  char						*_Name;
	// the parent timer
	CHTimer							*_Parent;
	// the sons timers
	TTimerVect						_Sons;
	// Tells if this is a root node
	bool							_IsRoot;
private:
	// root node of the hierarchy
	static CNode					_RootNode;
	// the current node of the execution
	static CNode					*_CurrNode;	
	// the root timer
	static CHTimer					 _RootTimer;
	/** This clock is used to measure the preambule of methods such as CHTimer::before()
	  * This is static, but the Hierarchical Timer doesn't support multithreading anyway..
      */
	static CSimpleClock				_PreambuleClock;
	//
	static double					_MsPerTick;
	//
	static bool						_Benching;
	//
	static bool						_BenchStartedOnce;
	//
	static bool						_WantStandardDeviation;
	//
	static CHTimer				   *_CurrTimer;	

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
	bool		_DisplayAfter;
public:
	CAutoTimer(CHTimer *timer, bool displayAfter=false) : _HTimer(timer), _DisplayAfter(displayAfter) { _HTimer->before(); }
	~CAutoTimer() { _HTimer->after(_DisplayAfter); }
};


////////////////////////////
// inlines implementation //
////////////////////////////
#if 0
//===============================================
inline void	CHTimer::before()
{	
	if (!_Benching) return;
	_PreambuleClock.start();
	walkTreeToCurrent();			
	++ _CurrNode->NumVisits;
	_CurrNode->SonsPreambule = 0;
	if (!_Parent && _CurrTimer != this)
	{
		_Parent = _CurrTimer;
		// register as a son of the parent
		_Parent->_Sons.push_back(this); 
	}
	_CurrTimer = this;
	_PreambuleClock.stop();
	if (_CurrNode->Parent)
	{	
		_CurrNode->Parent->SonsPreambule += _PreambuleClock.getNumTicks();
	}
	_CurrNode->Clock.start();
}

//===============================================
inline void	CHTimer::after(bool displayAfter /*= false*/)
{
	if (!_Benching) return;
	_CurrNode->Clock.stop();		
	_PreambuleClock.start();
	//		
	//nlinfo((std::string("clock ") + _Name + std::string(" time = ") + NLMISC::toString(_CurrNode->Clock.getNumTicks())).c_str());
	uint64 numTicks = _CurrNode->Clock.getNumTicks()  - _CurrNode->SonsPreambule - (CSimpleClock::getStartStopNumTicks() << 1);

	_CurrNode->TotalTime += numTicks;		
	_CurrNode->MinTime = std::min(_CurrNode->MinTime, numTicks);
	_CurrNode->MaxTime = std::max(_CurrNode->MaxTime, numTicks);
	_CurrNode->LastSonsTotalTime = _CurrNode->SonsTotalTime;
	if (displayAfter)
	{		
		nlinfo("FEHTIMER> %s %.3fms loop number %d", _Name, numTicks * _MsPerTick, _CurrNode->NumVisits);
	}
	//
	if (_WantStandardDeviation)
	{
		_CurrNode->Measures.push_back(numTicks * _MsPerTick);
	}
	//
	if (_Parent)
	{
		_CurrTimer = _Parent;
	}	
	//
	if (_CurrNode->Parent)
	{
		_PreambuleClock.stop();
		_CurrNode = _CurrNode->Parent;
		_CurrNode->SonsTotalTime += numTicks;
		_CurrNode->SonsPreambule += _PreambuleClock.getNumTicks();
	}
	else
	{
		_PreambuleClock.stop();
	}
}
#endif

} // NLMISC

#endif // NL_HIERARCHICAL_TIMER_H

/* End of hierarchical_timer.h */

