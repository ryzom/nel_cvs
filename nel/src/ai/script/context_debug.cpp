/** \file context_debug.cpp
 * Debug informations in the context.
 *
 * $Id: context_debug.cpp,v 1.8 2003/01/21 11:24:39 chafik Exp $
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

#include "nel/ai/script/context_debug.h"
#include "nel/ai/script/code_branche_run_debug.h"
#include "nel/ai/script/codage.h"

namespace NLAISCRIPT
{
	CContextDebug::CContextDebug(): HeapDebug(*(new CStackPointer()))
	{
		_DebugMode = stepByStepMode;
		Active = false;
		_StepIndex = 1;
		_LastCommandLine = new char[1024];
		_LastCommandLine[0] = ' ';
		_LastCommandLine[1] = '\0';
	}

	CContextDebug::~CContextDebug()
	{
		_CallStack.clear();
		delete _LastCommandLine;
		delete &HeapDebug;
		if (Param.size() >0 )
		{
			std::list<NLAIAGENT::IObjectIA	*>::const_iterator It = Param.begin();
			while(It != Param.end())
			{
				(*It)->release();
				It++;
			}
		}
	}

	const NLAIC::CIdentType &CContextDebug::getType() const
	{
		return IdContextDebug;
	}

	const NLAIC::IBasicType* CContextDebug::clone() const
	{
		NLAIC::IBasicType *x = new (CContextDebug);
		return x;
	}

	const NLAIC::IBasicType* CContextDebug::newInstance() const
	{
		return clone();
	}

	void CContextDebug::getDebugString(std::string &dbgStr) const
	{
	}

	void CContextDebug::save(NLMISC::IStream &os)
	{
	}

	void CContextDebug::load(NLMISC::IStream &is)
	{
	}

	bool CContextDebug::isEqual(const NLAIAGENT::IBasicObjectIA &a) const
	{
		return true;
	}

	const NLAIAGENT::IObjectIA::CProcessResult &CContextDebug::run()
	{		
		return NLAIAGENT::IObjectIA::ProcessRun;
	}
	void CContextDebug::setDebugMode (TDebugMode dm)
	{
		_DebugMode = dm;
	}

	TDebugMode CContextDebug::getDebugMode () const
	{
		return _DebugMode;
	}

	void CContextDebug::setStepIndex(uint16 si)
	{
		_StepIndex = si;
	}

	uint16 CContextDebug::getStepIndex () const
	{
		return _StepIndex;
	}

	uint16 CContextDebug::getCallStackTopIndex() const
	{
		return _CallStack.size();
	}

	void CContextDebug::callStackPop()
	{
		_CallStack.pop_back();
	}

	void CContextDebug::callStackPush(const CCodeBrancheRun* cbr)
	{
		_CallStack.push_back(cbr);
	}

	void CContextDebug::addBreakPoint(uint16 line, const char* fileName)
	{
		std::map<TMystring, TUintSet>::iterator itu;
		std::set<uint16>::iterator it;
		if ((itu = _BreakPointSet.find(fileName)) != _BreakPointSet.end())
		{
			if ((it = itu->second.find(line)) == itu->second.end())
			{
				itu->second.insert(line);
			}
		}
		else
		{
			_BreakPointSet[fileName].insert(line);
		}
	}

	void CContextDebug::eraseBreakPoint(uint16 line, const char* fileName)
	{
		std::map<TMystring, TUintSet>::iterator itu;
		std::set<uint16>::iterator it;
		if ((itu = _BreakPointSet.find(fileName)) != _BreakPointSet.end())
		{
			if ((it = itu->second.find(line)) != itu->second.end())
			{
				itu->second.erase(it);
			}
		}
	}

	bool CContextDebug::getBreakPointValue(uint16 line, const char* fileName) const
	{
		std::map<TMystring, TUintSet>::const_iterator itu;
		std::set<uint16>::iterator it;
		if (   (itu = _BreakPointSet.find(fileName)) != _BreakPointSet.end()
			&& (it = itu->second.find(line)) != itu->second.end() )
			return true;
		else
			return false;
	}

	void CContextDebug::callStackPrint(NLAIC::IIO *inputOutput) const
	{
		CCodeBrancheRunDebug* pCbrd;
		std::list<const CCodeBrancheRun*>::const_iterator itC = _CallStack.begin();
		while (itC != _CallStack.end())
		{
			if ((*itC)->getType() == CCodeBrancheRunDebug::IdCodeBrancheRunDebug)
			{
				pCbrd = (CCodeBrancheRunDebug*)(*itC);
				inputOutput->Echo("%s\n",pCbrd->getSourceFileName().c_str());
				pCbrd->printCurrentSourceCodeLine();
			}
			itC++;
		}
	}

	void CContextDebug::stepIndexUp()
	{
		if (_StepIndex < _CallStack.size())
		{
			_StepIndex ++;
		}
	}

	void CContextDebug::stepIndexDown()
	{
		if (_StepIndex > 0)
		{
			_StepIndex --;
		}
	}

	void CContextDebug::printActiveBeaks(NLAIC::IIO *inputOutput) const
	{
		std::map<TMystring, TUintSet>::const_iterator itu = _BreakPointSet.begin();
		std::set<uint16>::iterator it;
		while (itu != _BreakPointSet.end())
		{
			inputOutput->Echo("File name : %s\nLines : ", (*itu).first.c_str());
			it = (*itu).second.begin();
			while (it != (*itu).second.end())
			{
				inputOutput->Echo("%d ", (*it));
				it++;
			}
			inputOutput->Echo("\n");
			itu++;
		}
	}

	const char* CContextDebug::getLastCommandLine() const
	{
		return _LastCommandLine;
	}

	void CContextDebug::setLastCommandLine(const char* c)
	{
		strcpy(_LastCommandLine, c);
	}
}
