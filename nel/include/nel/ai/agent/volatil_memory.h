/** \file volatil_memory.h
 *
 * $Id: volatil_memory.h,v 1.8 2002/11/15 09:20:53 chafik Exp $
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
#include "nel/ai/agent/agent.h"
#include "nel/ai/agent/agent_object.h"
#include "nel/ai/agent/agent_timer.h"

#ifndef NL_VOLATIL_MEMORY_H
#define NL_VOLATIL_MEMORY_H

namespace NLAIAGENT
{	
	class CHashTimerManager: public IConnectIA
	{
	public:
		static const NLAIC::CIdentType *IdHashTimerManager;

	public:
		static sint MaxSubscribByTimer;
		static uint Time;

	private:
		std::list<CAgentWatchTimer *> TimerAccount;		

	public:
		CHashTimerManager() {}
		CHashTimerManager(const CHashTimerManager &);
		~CHashTimerManager() {}


		CAgentWatchTimer *getTimer();
		

		virtual void getDebugString(std::string &t) const
		{
			t += "HashTimerManager";
		}

		virtual const NLAIC::CIdentType &getType() const
		{
			return *IdHashTimerManager;
		}

		virtual void save(NLMISC::IStream &os)
		{
		}
		virtual void load(NLMISC::IStream &is)
		{
		}

		virtual const NLAIC::IBasicType *newInstance() const
		{			
			NLAIC::IBasicType *x = new CHashTimerManager();		
			return x;
		}

		virtual const NLAIC::IBasicType *clone() const
		{			
			NLAIC::IBasicType *x = new CHashTimerManager(*this);			
			return x;
		}

		virtual bool isEqual(const IBasicObjectIA &a) const
		{
			return this == &a;
		}

		virtual void onKill(IConnectIA *)
		{
		}

		virtual IObjectIA::CProcessResult sendMessage(IMessageBase *msg)
		{
			msg->release();
			return IObjectIA::CProcessResult();
		}

		virtual const CProcessResult &getState() const
		{
			return IObjectIA::ProcessRun;
		}

		virtual void setState(TProcessStatement s, IObjectIA *result) 
		{
		}
		virtual const CProcessResult &run()
		{
			return IObjectIA::ProcessRun;
		}

	public:
		static void initClass();
		static void releaseClass();

	};
	class CVolatilMemmory: public IConnectIA
	{
	public:		
		static CHashTimerManager *Timer;

	public:
		static const NLAIC::CIdentType *IdVolatilMemmory;

	protected:
		uint _Time;
		IBaseGroupType *_List;
		std::list<IObjectIA *> _Connecter; 
		const CPairType *_PairToDelete;

	
	public:
		CVolatilMemmory(IBaseGroupType *l = new CGroupType);
		CVolatilMemmory(const CVolatilMemmory &);
		virtual ~CVolatilMemmory();		


		CConstIteratorContener getConstIterator() const
		{
			return _List->getConstIterator();
		}

		void pushFront(uint time,IObjectIA *);
		void pushBack(uint time, IObjectIA *);

		void popFront();
		void popBack();
		IObjectIA *Front() const;
		IObjectIA *Back() const;
		void erase(CConstIteratorContener &);

		int size()
		{
			return _List->size();
		}

		void addAccount(IObjectIA *);
		void releaseAccount(IObjectIA *);
		void init();

		/// \name IBasicInterface method.
		//@{
		virtual void save(NLMISC::IStream &os)
		{
		}
		virtual void load(NLMISC::IStream &is)
		{
		}
		
		virtual const NLAIC::CIdentType &getType() const
		{
			return *IdVolatilMemmory;
		}

		virtual const NLAIC::IBasicType *newInstance() const
		{			
			NLAIC::IBasicType *x = new CVolatilMemmory();		
			return x;
		}

		virtual const NLAIC::IBasicType *clone() const
		{			
			NLAIC::IBasicType *x = new CVolatilMemmory(*this);			
			return x;
		}

		virtual void getDebugString(std::string &t) const;

		virtual bool isEqual(const IBasicObjectIA &a) const
		{
			return this == &a;
		}

		virtual void onKill(IConnectIA *);	

		virtual IObjectIA::CProcessResult sendMessage(IMessageBase *msg);

		virtual const CProcessResult &getState() const
		{
			return IObjectIA::ProcessRun;
		}
		virtual void setState(TProcessStatement s, IObjectIA *result) 
		{
		}

		//@}

		/// \name IObjectIA method.
		//@{
		virtual const CProcessResult &run()
		{
			return IObjectIA::ProcessRun;
		}
		//@}

	protected:
		virtual void sendUpdateMessage(IObjectIA *);

	private:
		void searchBest();
		void testIfBest(const CPairType *);
		void runMessage(CAgentWatchTimer *);

	public:
		static void initClass();
		static void releaseClass();
	};
}
#endif
