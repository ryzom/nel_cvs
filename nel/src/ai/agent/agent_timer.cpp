/** \file agent_timer.cpp
 *
 * $Id: agent_timer.cpp,v 1.2 2001/04/19 13:45:09 chafik Exp $
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
#include "nel/ai/nl_ai.h"
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

	const int CAgentManagerTimer::ClockTick = 20;

	void CAgentManagerTimer::initClass()

	{
		CAgentManagerTimer *h = new CAgentManagerTimer(NULL);
		CAgentManagerTimer::IdAgentTimer = new NLAIC::CIdentType ("AgentStaticManagerTimer", NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)*h), 
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
			if(time > (uint)CAgentManagerTimer::ClockTick)
			{
				start = NLMISC::CTime::getLocalTime();
				NLMISC::CSynchronized<CAgentScript *>::CAccessor accessor(CAgentManagerTimer::TimerManager);
				accessor.value()->run();
			}
			NLMISC::nlSleep(CAgentManagerTimer::ClockTick);
		}
	}

//##################################
//CLibTimerManager
//##################################
	const NLAIC::CIdentType *CLibTimerManager::IdAgentManagerTimer = NULL;
	NLAIAGENT::CAgentScript::CMethodCall **CLibTimerManager::StaticMethod = NULL;

	NLAIAGENT::tQueue CLibTimerManager::isMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *methodName,const NLAIAGENT::IObjectIA &param) const
	{
		NLAIAGENT::tQueue r = NLAIAGENT::isTemplateMember(CLibTimerManager::StaticMethod,CLibTimerManager::TLastM,IObjectIA::getMethodIndexSize(),className,methodName,param);
		if(r.size()) return r;
		else return IObjectIA::isMember(className,methodName,param);
	}
	

	NLAIAGENT::IObjectIA::CProcessResult CLibTimerManager::runMethodeMember(sint32 index,NLAIAGENT::IObjectIA *o)
	{
		NLAIAGENT::IBaseGroupType *param = (NLAIAGENT::IBaseGroupType *)o;

		switch(index - IObjectIA::getMethodIndexSize())
		{
		case CLibTimerManager::TGetTimer:
			{												
				NLAIAGENT::IObjectIA::CProcessResult a;
				{					
					NLMISC::CSynchronized<CAgentScript *>::CAccessor accessor(CAgentManagerTimer::TimerManager);
					a = accessor.value()->getDynamicAgent(param);
				}				
				return a;
			}		
		default:
			return IObjectIA::runMethodeMember(index,o);
		}
	}

	void CLibTimerManager::initClass()
	{
		CLibTimerManager h;
		CLibTimerManager::IdAgentManagerTimer = new NLAIC::CIdentType ("AgentManagerTimer", NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)h), 
																	  NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgent),
																	  NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));

		NLAISCRIPT::CParam *ParamString = new NLAISCRIPT::CParam(1,new NLAISCRIPT::COperandSimple(new NLAIC::CIdentType(NLAIAGENT::CStringType::IdStringType)));
		CLibTimerManager::StaticMethod = new NLAIAGENT::CAgentScript::CMethodCall *[CLibTimerManager::TLastM];
		
		CLibTimerManager::StaticMethod[CLibTimerManager::TGetTimer] = 
							new NLAIAGENT::CAgentScript::CMethodCall(_GETTIMER_,
																	CLibTimerManager::TGetTimer,
																	ParamString,NLAIAGENT::CAgentScript::CheckAll,
																	1,	new NLAISCRIPT::CObjectUnknown(
																		new NLAISCRIPT::COperandSimple(
																		new NLAIC::CIdentType(CVectorGroupManager::IdVectorGroupManager))));
	}

	void CLibTimerManager::releaseClass()
	{
		delete CLibTimerManager::IdAgentManagerTimer;
		int i;
		for(i = 0; i < CLibTimerManager::TLastM; i++)
		{
			delete CLibTimerManager::StaticMethod[i];
		}
		delete CLibTimerManager::StaticMethod;
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
			tellBroker();
			setState(processToKill,NULL);			
		}
		return getState();
	}

	void CAgentWatchTimer::tellBroker()
	{
		IMessageBase *msg = (IMessageBase *)_MSG->clone();
		msg->setSender(this);
		msg->setPerformatif(IMessageBase::PTell);
		_Call->sendMessage(msg);
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
		CVectorGroupType g(2);
		char t[256*4];
		((const IWordNumRef &)*this).getNumIdent().getDebugString(t);		
		g.set(0,new CStringType(CStringVarName(t)));
		this->incRef();
		g.set(1,this);
		NLMISC::CSynchronized<CAgentScript *>::CAccessor accessor(CAgentManagerTimer::TimerManager);
		accessor.value()->addDynamicAgent(&g);
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
				CIteratorContener i = param->getIterator();
				IObjectIA *call = (IObjectIA *)i++;
				call->incRef();
				IMessageBase *msg = (IMessageBase *)i++;
				msg->incRef();
				setAttrib(call,msg);
				attach();
				IObjectIA::CProcessResult a;				
				char t[256*4];
				((const IWordNumRef &)*this).getNumIdent().getDebugString(t);
				a.Result = new CStringType(CStringVarName(t));
				return a;
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
	IMessageBase *CAgentWatchTimer::runTell(const IMessageBase &m)
	{
		static NLAIC::CIdentType idMsgKillTimer ("MsgStopTimer");
		static NLAIC::CIdentType idMsgStartTimer ("MsgStartTimer");
		if(m.getType() == idMsgKillTimer)
		{
			setState(processEnd,NULL);
			return NULL;
		}
		else
		if(m.getType() == idMsgStartTimer)
		{
			setState(processIdle,NULL);
			return NULL;
		}
		else return CAgentScript::runTell(m);


	}

	NLAIAGENT::CAgentScript::CMethodCall **CAgentWatchTimer::StaticMethod = NULL;

	void CAgentWatchTimer::initMsgClass()
	{
		std::string msgStr;
		std::string scriptName("MsgTimer");
		msgStr = std::string("From Message : Define MsgStopTimer\n{");
		msgStr += std::string("Component:\n");		
		msgStr += std::string("End\n}\n");		
		NLAILINK::buildScript(msgStr,scriptName);
		msgStr = std::string("From Message : Define MsgStartTimer\n{");
		msgStr += std::string("Component:\n");
		msgStr += std::string("End\n}\n");		
		NLAILINK::buildScript(msgStr,scriptName);
	}
	void CAgentWatchTimer::initClass()
	{
		initMsgClass();
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
																	2,	new NLAISCRIPT::CObjectUnknown(	new NLAISCRIPT::COperandSimple(
																		new NLAIC::CIdentType(NLAIAGENT::CStringType::IdStringType))));
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
		int i;
		for(i = 0; i < CAgentWatchTimer::TLastM; i++)
		{
			delete CAgentWatchTimer::StaticMethod[i];
		}
		delete CAgentWatchTimer::StaticMethod;
	}

//##################################
//CAgentClockTimer
//##################################

	const NLAIC::CIdentType *CAgentClockTimer::IdAgentClockTimer = NULL;

	CAgentClockTimer::CAgentClockTimer():CAgentWatchTimer(), _TimeCount(0)
	{
	}

	CAgentClockTimer::CAgentClockTimer(IAgentManager *m):CAgentWatchTimer(m), _TimeCount(0)
	{
	}

	CAgentClockTimer::CAgentClockTimer(IAgentManager *m, IBasicAgent *a, std::list<IObjectIA *> &v, NLAISCRIPT::CAgentClass *c):
					CAgentWatchTimer(m,a,v,c), _TimeCount(0)
	{
	}

	CAgentClockTimer::CAgentClockTimer(const CAgentClockTimer &t):CAgentWatchTimer(t), _TimeCount(t._TimeCount)
	{
	}

	CAgentClockTimer::~CAgentClockTimer()
	{
	}

	const NLAIC::IBasicType *CAgentClockTimer::clone() const
	{
		return new CAgentClockTimer(*this);
	}
	const NLAIC::IBasicType *CAgentClockTimer::newInstance() const
	{
		return new CAgentClockTimer();
	}
	const NLAIC::CIdentType &CAgentClockTimer::getType() const
	{
		return *IdAgentClockTimer;
	}

	IObjectIA::CProcessResult CAgentClockTimer::runActivity()
	{
		_Clock -= CAgentManagerTimer::ClockTick;
		if(_Clock <= 0)
		{
			_Clock = _TimeCount;
			tellBroker();
		}
		return getState();
	}

	void CAgentClockTimer::initClass()
	{
		CAgentClockTimer h;
		CAgentClockTimer::IdAgentClockTimer = new NLAIC::CIdentType ("AgentClockTimer", NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)h), 
																	  NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tAgent),
																	  NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));
	}

	void CAgentClockTimer::releaseClass()
	{
		delete CAgentClockTimer::IdAgentClockTimer;
	}
}