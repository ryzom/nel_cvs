/** \file context_debug.h
 * Debug informations in the context.
 *
 * $Id: context_debug.h,v 1.8 2003/01/21 11:24:25 chafik Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX D.T.C. SYSTEM.
 * NEVRAX D.T.C. SYSTEM is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX D.T.C. SYSTEM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX D.T.C. SYSTEM; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02111-1307, USA.
 */

#ifndef NL_CONTEXT_DEBUG_H
#define NL_CONTEXT_DEBUG_H

#include <set>
#include <string>
#include "nel/ai/agent/agentpack.h"
#include "nel/ai/agent/agent_script.h"

namespace NLAISCRIPT
{
	enum TDebugMode
	{
		runMode,
		stepByStepMode,
		stepInMode,
		stepOutMode,
		stopMode
	};
	class CCodeBrancheRun;
	class CStackPointer;

	/**
	 * Debug informations in the context.
	 * \author Gabriel ROBERT
	 * \author Nevrax France
	 * \date 2000
	 */
	class CContextDebug : public NLAIAGENT::IObjectIA
	{
	public:
		static const NLAIC::CIdentType IdContextDebug;

		/// Are we in a debug context ?
		bool									Active;

		/// Name of the variable on the heap.
		CStackPointer							&HeapDebug; 

		// Var name of the current function.
		std::list<NLAIAGENT::IObjectIA	*>				Param;	

	private :
		NLAIAGENT::TProcessStatement					_RunState;
		std::list<const CCodeBrancheRun*>		_CallStack;
		TDebugMode								_DebugMode;
		uint16									_StepIndex; // Index of the current StepByStep branche in the _CallStack
		typedef std::set<uint16> TUintSet;
		typedef std::string TMystring;
		std::map<TMystring, TUintSet>				_BreakPointSet;
		char									*_LastCommandLine;

	public :
		/// Constructor.
		CContextDebug();

		/// Destructor.
		virtual ~CContextDebug();

		/// Return the class type.
		const NLAIC::CIdentType &getType() const;

		/// Return a copy of this object.
		const NLAIC::IBasicType* clone() const;

		/// Idem as clone.
		const NLAIC::IBasicType* newInstance() const;

		/// Return a debugSting.
		void getDebugString(std::string &dbgStr) const;

		/// Save the object in a stream
		virtual void save(NLMISC::IStream &os);

		/// Load the object from a stream.
		void load(NLMISC::IStream &is);

		/// Are two object egals ?
		bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;

		/// Run current obect.
		const NLAIAGENT::IObjectIA::CProcessResult &run();

		/// Debug mode could be run, steb by step, step in, step out or stop.
		//@{
		void setDebugMode(TDebugMode dm);
		TDebugMode getDebugMode () const;
		//@}

		/// Index of the current StepByStep branche in the _CallStack.
		//@{
		void setStepIndex(uint16 si);
		uint16 getStepIndex () const;
		//@}

		/// Incrase the StepIndex
		void stepIndexUp();

		/// Decrase the StepIndex
		void stepIndexDown();

		/// Return the index of the last CodebrancheRun in the _CallStack.
		uint16 getCallStackTopIndex() const;

		/// Pop the last CodebrancheRun in the _CallStack.
		void callStackPop();

		/// Push a CCodeBrancheRun on the _CallStack.
		void callStackPush(const CCodeBrancheRun* cbr);

		/// Print the current call stack.
		void callStackPrint(NLAIC::IIO *inputOutput) const;

		/// Add a BreakPoint.
		void addBreakPoint(uint16 line, const char* fileName);

		/// Remove a BreakPoint.
		void eraseBreakPoint(uint16 line, const char* fileName);

		/// Return the breakPointValue.
		bool getBreakPointValue(uint16 line, const char* fileName) const;

		/// Liste active breaks.
		void printActiveBeaks(NLAIC::IIO *inputOutput) const;

		/// Get the last command Line.
		const char* getLastCommandLine() const;

		/// Store the last command Line.
		void setLastCommandLine(const char* c);

		///For reinit the context after using.
		void init()
		{
			if(Active) _DebugMode = stepByStepMode;
		}
	};
}

#endif // NL_CONTEXT_DEBUG_H

/* End of context_debug.h */
