/** \file agent_timer.h
 * class for string manipulation.
 *
 * $Id: agent_timer.h,v 1.14 2003/01/23 15:40:56 chafik Exp $
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
#include "nel/misc/time_nl.h"

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
	
		static bool IsRunning;
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
		virtual void getDebugString(std::string &t) const;
		virtual const NLAIC::CIdentType &getType() const;
		//@}
	public:
		static void initClass();
		static void releaseClass();
	};


	class CLibTimerManager: public NLAIAGENT::IObjectIA
	{
	public:
		///This enum define ident for hard coded method that we have to import its under the script.
		enum {			
			TGetTimer,
			TLastM ///The count of export method.
		};
		static NLAIAGENT::CAgentScript::CMethodCall **StaticMethod;

	public:
		static const NLAIC::CIdentType *IdAgentManagerTimer;
	public:
		CLibTimerManager() {}

		virtual sint32 getMethodIndexSize() const
		{
			return IObjectIA::getMethodIndexSize() + 0;
		}
		
		virtual NLAIAGENT::TQueue isMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *mathodName,const NLAIAGENT::IObjectIA &) const;		
		virtual	NLAIAGENT::IObjectIA::CProcessResult runMethodeMember(sint32 index,NLAIAGENT::IObjectIA *);

		const NLAIAGENT::IObjectIA::CProcessResult &run(){return NLAIAGENT::IObjectIA::ProcessRun;}
		bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const{ return true;}	

		/// \name NLAIC::IBasicInterface method.
		//@{
		const NLAIC::IBasicType *clone() const
		{
			NLAIC::IBasicType *x = new CLibTimerManager();
			return x;            
		}

		const NLAIC::IBasicType *newInstance() const 
		{
			return clone();
		}

		const NLAIC::CIdentType &getType() const
		{
			return *IdAgentManagerTimer;
		}

		void getDebugString(std::string &t) const
		{
			t += "lib for Timer access manager";
		}

		void save(NLMISC::IStream &os)
		{
			
		}

		void load(NLMISC::IStream &is) 
		{				
		}
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
		TAddAttrib,
		TLastM
		};
		
		static NLAIAGENT::CAgentScript::CMethodCall **StaticMethod;

	public:
		static const NLAIC::CIdentType *IdAgentWatchTimer;

	protected:

		sint _Clock;
		NLMISC::TTicks _DTime;
		std::list<std::pair< IConnectIA *, std::pair<IMessageBase *, sint32> > > _Call;
		std::list<std::pair< IConnectIA *, std::pair<IMessageBase *, sint32> > >::iterator _CallIter;
		/*_Call;
		_MSG;*/
		
	public:
		CAgentWatchTimer();
		CAgentWatchTimer(IAgentManager *);
		CAgentWatchTimer(IAgentManager *, IBasicAgent *, std::list<IObjectIA *> &, NLAISCRIPT::CAgentClass *);
		CAgentWatchTimer(const CAgentWatchTimer &);
		virtual ~CAgentWatchTimer();

		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const;
		virtual void getDebugString(std::string &t) const;
		virtual void onKill(IConnectIA *A);

		virtual sint getClock() const
		{
			return _Clock;
		}

		virtual void setClock(uint c)
		{
			_Clock = c;
		}

		void addAttrib(IConnectIA *,IMessageBase *);
		void attach();
		void detach();
		bool detach(IConnectIA *,bool deleteFromConnection = true);
		bool tellBroker();
		sint getSubscribedCount() const
		{
			return _Call.size();
		}

		virtual IObjectIA::CProcessResult runActivity();

		virtual bool haveActivity() const
		{
			return true;
		}


		virtual int getBaseMethodCount() const;
		virtual NLAIAGENT::TQueue isMember(const NLAIAGENT::IVarName *,const NLAIAGENT::IVarName *,const NLAIAGENT::IObjectIA &) const;
		virtual sint32 getMethodIndexSize() const;
		virtual IObjectIA::CProcessResult runMethodBase(int heritance, int index,NLAIAGENT::IObjectIA *);
		virtual IObjectIA::CProcessResult runMethodBase(int index,NLAIAGENT::IObjectIA *);
		virtual IObjectIA::CProcessResult runMethodeMember(sint32 index,NLAIAGENT::IObjectIA *);

		virtual IMessageBase *runTell(const IMessageBase &m);
		virtual IMessageBase *runKill(const IMessageBase &m)
		{
			setState(processToKill,NULL);
			return NULL;
		}

	public:
		static void initClass();
		static void initMsgClass();
		static void releaseClass();
	};

	class CAgentClockTimer: public CAgentWatchTimer
	{

	public:
		static const NLAIC::CIdentType *IdAgentClockTimer;
	private:
		sint _TimeCount;
	public:
		CAgentClockTimer();
		CAgentClockTimer(IAgentManager *);
		CAgentClockTimer(IAgentManager *, IBasicAgent *, std::list<IObjectIA *> &, NLAISCRIPT::CAgentClass *);
		CAgentClockTimer(const CAgentClockTimer &);
		virtual ~CAgentClockTimer();

		virtual void setClock(uint c);		

		virtual sint getClock() const
		{
			return _TimeCount;
		}

		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const;

		virtual IObjectIA::CProcessResult runActivity();

	public:
		static void initClass();
		static void releaseClass();


	};

	class CAgentTimerHandle: public IObjectIA
	{
	public:
		static const NLAIC::CIdentType *IdAgentTimerHandle;

	private:		
		CAgentWatchTimer *_Timer;

	public:

		CAgentTimerHandle();
		CAgentTimerHandle(CAgentWatchTimer *);
		CAgentTimerHandle(const CAgentTimerHandle &);
		virtual ~CAgentTimerHandle();

		virtual const NLAIC::IBasicType *clone() const;
		virtual const NLAIC::IBasicType *newInstance() const;
		virtual const NLAIC::CIdentType &getType() const;
		virtual void getDebugString(std::string &t) const;
		virtual void save(NLMISC::IStream &os);		
		virtual void load(NLMISC::IStream &is);
		virtual const NLAIAGENT::IObjectIA::CProcessResult &run(){return NLAIAGENT::IObjectIA::ProcessRun;}
		virtual bool isEqual(const NLAIAGENT::IBasicObjectIA &a) const;
		virtual IObjectIA::CProcessResult sendMessage(IObjectIA *m);

		virtual sint32 getMethodIndexSize() const;		
		virtual TQueue isMember(const IVarName *h,const IVarName *m,const IObjectIA &p) const;
		virtual	CProcessResult runMethodeMember(sint32 h, sint32 m, IObjectIA *p);
		virtual	CProcessResult runMethodeMember(sint32 m,IObjectIA *p);		

	public:
		static void initClass();
		static void releaseClass();
	};
}
#endif
