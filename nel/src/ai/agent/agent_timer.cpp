/** \file agent_timer.cpp
 *
 * $Id: agent_timer.cpp,v 1.1 2001/04/19 08:13:12 chafik Exp $
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
#include "nel/misc/time_nl.h"
#include "nel/ai/agent/agent_timer.h"
#include "nel/ai/agent/agent_method_def.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/script/object_unknown.h"

namespace NLAIAGENT
{
//##################################
//CAgentManagerTimer
//##################################
	CAgentManagerTimer::CAgentManagerTimer(const CAgentManagerTimer &t):CAgentScript(t), _Time(t._Time)
	{

	}

	CAgentManagerTimer::CAgentManagerTimer(IAgentManager *m):CAgentScript(m), _Time(0)
	{
	}

	CAgentManagerTimer::CAgentManagerTimer(IAgentManager *m, IBasicAgent *a, std::list<IObjectIA *> &v, NLAISCRIPT::CAgentClass *c):CAgentScript(m,a,v,c), _Time(0)
	{
	}

	CAgentManagerTimer::~CAgentManagerTimer()
	{
	}

	void CAgentManagerTimer::load(NLMISC::IStream &is)
	{
	}
	void CAgentManagerTimer::save(NLMISC::IStream &os)
	{
	}
	const NLAIC::IBasicType *CAgentManagerTimer::clone() const
	{
		return new CAgentManagerTimer(*this);
	}
	const NLAIC::IBasicType *CAgentManagerTimer::newInstance() const
	{
		return new CAgentManagerTimer(NULL);
	}

	void CAgentManagerTimer::getDebugString(char *t) const
	{
		sprintf(t,"CAgentManagerTimer <%d>",_Time);
	}

	const NLAIC::CIdentType &CAgentManagerTimer::getType() const
	{
		return *IdAgentTimer;
	}

	IObjectIA::CProcessResult CAgentManagerTimer::runActivity()
	{
		_Time ++;
		return IObjectIA::CProcessResult();
	}
	

	const NLAIC::CIdentType *CAgentManagerTimer::IdAgentTimer =  NULL;
	NLMISC::CSynchronized<CAgentScript *> *CAgentManagerTimer::TimerManager = NULL;
	NLMISC::IThread *CAgentManagerTimer::TimerManagerRun = NULL;	
	CAgentManagerTimer::CRunTimer *CAgentManagerTimer::RunTimer = NULL;

	const int CAgentManagerTimer::ClockTick = 40;

	void CAgentManagerTimer::initClass()

	{
		CAgentManagerTimer *h = new CAgentManagerTimer(NULL);
		CAgentManagerTimer::IdAgentTimer = new NLAIC::CIdentType ("AgentManagerTimer", NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)*h), 
															NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgent),
															NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

		CAgentManagerTimer::TimerManager = new NLMISC::CSynchronized<CAgentScript *>;
		{
			NLMISC::CSynchronized<CAgentScript *>::CAccessor accessor(CAgentManagerTimer::TimerManager);
			accessor.value() = h;
		}
		CAgentManagerTimer::RunTimer = new CAgentManagerTimer::CRunTimer();
		CAgentManagerTimer::TimerManagerRun =  NLMISC::IThread::create(CAgentManagerTimer::RunTimer);
		CAgentManagerTimer::TimerManagerRun->start();
	}

	void CAgentManagerTimer::releaseClass()
	{
		delete CAgentManagerTimer::IdAgentTimer;
		CAgentManagerTimer::IdAgentTimer = NULL;
		CAgentManagerTimer::TimerManagerRun->terminate();		
		{
			NLMISC::CSynchronized<CAgentScript *>::CAccessor accessor(CAgentManagerTimer::TimerManager);
			delete accessor.value();
		}
		delete CAgentManagerTimer::TimerManager;
	}

//##################################
//CAgentManagerTimer::CRunTimer
//##################################

	void CAgentManagerTimer::CRunTimer::run()
	{

		NLMISC::TTime start = NLMISC::CTime::getLocalTime();
		NLMISC::TTime time;
		while(1)
		{
			NLMISC::TTime t= NLMISC::CTime::getLocalTime();
			time = (t - start);
			if(time > 40)
			{
				start = NLMISC::CTime::getLocalTime();
				NLMISC::CSynchronized<CAgentScript *>::CAccessor accessor(CAgentManagerTimer::TimerManager);
				accessor.value()->run();
			}
			NLMISC::nlSleep(CAgentManagerTimer::ClockTick);
		}
	}
//##################################
//CAgentWatchTimer
//##################################
	const NLAIC::CIdentType *CAgentWatchTimer::IdAgentWatchTimer = NULL;
	

	CAgentWatchTimer::CAgentWatchTimer(): CAgentScript(NULL),_Clock(0),_Call(NULL),_MSG(NULL)
	{
	}
	CAgentWatchTimer::CAgentWatchTimer(const CAgentWatchTimer &t):CAgentScript(t), _Clock(t._Clock),_Call(t._Call),_MSG(t._MSG)
	{
		if(_Call != NULL) _Call->incRef();
		if(_MSG != NULL) _MSG->incRef();
	}
	CAgentWatchTimer::CAgentWatchTimer(IAgentManager *m):CAgentScript(m), _Clock(0),_Call(NULL),_MSG(NULL)
	{
	}
	CAgentWatchTimer::CAgentWatchTimer(IAgentManager *m, IBasicAgent *a, std::list<IObjectIA *> &v, NLAISCRIPT::CAgentClass *c):CAgentScript(m,a,v,c), _Clock(0),_Call(NULL),_MSG(NULL)
	{
	}
	CAgentWatchTimer::~CAgentWatchTimer()
	{
		if(_Call != NULL) _Call->release();
		if(_MSG != NULL) _MSG->release();
	}
	
	const NLAIC::IBasicType *CAgentWatchTimer::clone() const
	{
		return new CAgentWatchTimer(*this);
	}

	const NLAIC::IBasicType *CAgentWatchTimer::newInstance() const
	{
		return new CAgentWatchTimer();
	}

	const NLAIC::CIdentType &CAgentWatchTimer::getType() const
	{
		return *IdAgentWatchTimer;
	}

	IObjectIA::CProcessResult CAgentWatchTimer::runActivity()
	{
		_Clock -= CAgentManagerTimer::ClockTick;
		if(_Clock <= 0)
		{				
			IMessageBase *msg = (IMessageBase *)_MSG->clone();
			msg->setSender(this);
			msg->setPerformatif(IMessageBase::PTell);
			_Call->sendMessage(msg);
			setState(processEnd,NULL);
			return getState();
		}
		return IObjectIA::CProcessResult();
	}

	void CAgentWatchTimer::setAttrib(IObjectIA *c,IMessageBase *m)
	{
		if(c != _Call)
		{
			if(_Call != NULL) _Call->release();
			_Call = c;
		}

		if(m != _MSG)
		{
			if(_MSG != NULL) _MSG->release();
			_MSG = m;
		}
	}

	void CAgentWatchTimer::attach()
	{		
		NLMISC::CSynchronized<CAgentScript *>::CAccessor accessor(CAgentManagerTimer::TimerManager);
		accessor.value()->addChild(this);
	}

	int CAgentWatchTimer::getBaseMethodCount() const
	{
		return CAgentScript::getBaseMethodCount() + CAgentWatchTimer::TLastM;
	}


	sint32 CAgentWatchTimer::getMethodIndexSize() const
	{
		return CAgentScript::getMethodIndexSize() + CAgentWatchTimer::TLastM;
	}

	

	NLAIAGENT::tQueue CAgentWatchTimer::isMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *methodName,const NLAIAGENT::IObjectIA &param) const
	{
		NLAIAGENT::tQueue r = NLAIAGENT::isTemplateMember(CAgentWatchTimer::StaticMethod,CAgentWatchTimer::TLastM,CAgentScript::getMethodIndexSize(),className,methodName,param);
		if(r.size()) return r;
		else return CAgentScript::isMember(className,methodName,param);
	}		

	NLAIAGENT::IObjectIA::CProcessResult CAgentWatchTimer::runMethodBase(int heritance, int index,NLAIAGENT::IObjectIA *o)
	{		
		return runMethodBase(index,o);
	}

	IObjectIA::CProcessResult CAgentWatchTimer::runMethodeMember(sint32 index,NLAIAGENT::IObjectIA *o)
	{
		return runMethodBase(index, o);
	}

	NLAIAGENT::IObjectIA::CProcessResult CAgentWatchTimer::runMethodBase(int index,NLAIAGENT::IObjectIA *o)
	{
		NLAIAGENT::IBaseGroupType *param = (NLAIAGENT::IBaseGroupType *)o;

		switch(index - CAgentScript::getMethodIndexSize())
		{
		case CAgentWatchTimer::TAttach:
			{
				//addZone((CZoneType &)*param->get());
				CIteratorContener i = param->getIterator();
				IObjectIA *call = (IObjectIA *)i++;
				call->incRef();
				IMessageBase *msg = (IMessageBase *)i++;
				msg->incRef();
				setAttrib(call,msg);
				attach();
				return NLAIAGENT::IObjectIA::CProcessResult();
			}
		case CAgentWatchTimer::TSetClock:
			{
				setClock( (uint)((NLAIAGENT::INombreDefine *)param->get())->getNumber() );
				return NLAIAGENT::IObjectIA::CProcessResult();
			}
		case CAgentWatchTimer::TGetClock:
			{
				NLAIAGENT::IObjectIA::CProcessResult r;
				r.Result = new NLAIAGENT::DDigitalType((double)getClock());
				return r;
			}
		default:
			return CAgentScript::runMethodBase(index,o);
		}
	}

	NLAIAGENT::CAgentScript::CMethodCall **CAgentWatchTimer::StaticMethod = NULL;

	void CAgentWatchTimer::initClass()
	{
		CAgentWatchTimer h;
		CAgentWatchTimer::IdAgentWatchTimer = new NLAIC::CIdentType ("AgentWatchTimer", NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)h), 
																	  NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgent),
																	  NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

		NLAISCRIPT::CParam *Param = new NLAISCRIPT::CParam(1, new NLAISCRIPT::COperandSimpleListOr(2,
																new NLAIC::CIdentType(NLAIAGENT::DDigitalType::IdDDigitalType),
																new NLAIC::CIdentType(NLAIAGENT::DigitalType::IdDigitalType)));


		CAgentWatchTimer::StaticMethod = new NLAIAGENT::CAgentScript::CMethodCall *[CAgentWatchTimer::TLastM];
		
		CAgentWatchTimer::StaticMethod[CAgentWatchTimer::TAttach] = 
							new NLAIAGENT::CAgentScript::CMethodCall(_ATTACH_,
																	CAgentWatchTimer::TAttach,
																	NULL,NLAIAGENT::CAgentScript::CheckCount,
																	2, new NLAISCRIPT::CObjectUnknown(new NLAISCRIPT::COperandVoid));
		CAgentWatchTimer::StaticMethod[CAgentWatchTimer::TGetClock] = 
							new NLAIAGENT::CAgentScript::CMethodCall(_GETCLOCK_,
																	CAgentWatchTimer::TGetClock,
																	NULL,NLAIAGENT::CAgentScript::CheckCount,
																	0, new NLAISCRIPT::CObjectUnknown(
																					new NLAISCRIPT::COperandSimple(
																					new NLAIC::CIdentType(NLAIAGENT::DigitalType::IdDigitalType))));

		CAgentWatchTimer::StaticMethod[CAgentWatchTimer::TSetClock] = 
							new NLAIAGENT::CAgentScript::CMethodCall(_SETCLOCK_,
																	CAgentWatchTimer::TSetClock,
																	Param,NLAIAGENT::CAgentScript::CheckAll,
																	1, new NLAISCRIPT::CObjectUnknown(
																					new NLAISCRIPT::COperandSimple(
																					new NLAIC::CIdentType(NLAIAGENT::DigitalType::IdDigitalType))));
	}

	void CAgentWatchTimer::releaseClass()
	{
		delete CAgentWatchTimer::IdAgentWatchTimer;
	}
}