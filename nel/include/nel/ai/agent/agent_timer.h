/** \file agent_timer.h
 * class for string manipulation.
 *
 * $Id: agent_timer.h,v 1.1 2001/04/19 08:43:21 chafik Exp $
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
#ifndef NL_AGENT_TIMER_H
#define NL_AGENT_TIMER_H
#include <list>

#include "nel/ai/agent/agent_script.h"
#include "nel/misc/thread.h"

namespace NLAIAGENT
{
	class CAgentManagerTimer: public CAgentScript
	{	

	public:
		class CRunTimer: public NLMISC::IRunnable
		{	
		public:
			CRunTimer(){}
			~CRunTimer(){}
			void run();
		};

	public:
	
		static NLMISC::CSynchronized<CAgentScript *> *TimerManager;
		static NLMISC::IThread *TimerManagerRun;
		static CRunTimer *RunTimer;
		
	public:
		static const NLAIC::CIdentType *IdAgentTimer;
		static const int ClockTick;

	private:
		
		uint _Time;

	public:		
		CAgentManagerTimer(const CAgentManagerTimer &);
		CAgentManagerTimer(IAgentManager *);
		CAgentManagerTimer(IAgentManager *, IBasicAgent *, std::list<IObjectIA *> &, NLAISCRIPT::CAgentClass *);
		virtual ~CAgentManagerTimer();	
		
		virtual IObjectIA::CProcessResult runActivity();
		virtual bool haveActivity() const
		{
			return true;
		}			
		
		
		/// \name NLAIC::IBasicInterface base class method.
		//@{
		virtual void load(NLMISC::IStream &is);
		virtual void save(NLMISC::IStream &os);
		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual void getDebugString(char *t) const;
		virtual const NLAIC::CIdentType &getType() const;
		//@}
	public:
		static void initClass();
		static void releaseClass();
	};

	class CAgentWatchTimer: public CAgentScript
	{
	public:				
		enum{		
		TAttach,
		TSetClock,
		TGetClock,
		TLastM
		};
		
		static NLAIAGENT::CAgentScript::CMethodCall **StaticMethod;

	public:
		static const NLAIC::CIdentType *IdAgentWatchTimer;

	private:

		int _Clock;
		IObjectIA *_Call;
		IMessageBase *_MSG;
		
	public:
		CAgentWatchTimer();
		CAgentWatchTimer(IAgentManager *);
		CAgentWatchTimer(IAgentManager *, IBasicAgent *, std::list<IObjectIA *> &, NLAISCRIPT::CAgentClass *);
		CAgentWatchTimer(const CAgentWatchTimer &);
		virtual ~CAgentWatchTimer();

		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const;

		uint getClock() const
		{
			return _Clock;

		}

		void setClock(uint c)
		{
			_Clock = c;
		}

		void setAttrib(IObjectIA *,IMessageBase *);
		void attach();

		virtual IObjectIA::CProcessResult runActivity();

		virtual bool haveActivity() const
		{
			return true;
		}


		virtual int getBaseMethodCount() const;
		virtual NLAIAGENT::tQueue isMember(const NLAIAGENT::IVarName *,const NLAIAGENT::IVarName *,const NLAIAGENT::IObjectIA &) const;
		virtual sint32 getMethodIndexSize() const;
		virtual IObjectIA::CProcessResult runMethodBase(int heritance, int index,NLAIAGENT::IObjectIA *);
		virtual IObjectIA::CProcessResult runMethodBase(int index,NLAIAGENT::IObjectIA *);
		virtual IObjectIA::CProcessResult runMethodeMember(sint32 index,NLAIAGENT::IObjectIA *);

	public:
		static void initClass();
		static void releaseClass();
	};
}
#endif