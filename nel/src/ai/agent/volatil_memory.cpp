/** \file volatil_memory.cpp
 *
 * $Id: volatil_memory.cpp,v 1.6 2002/11/15 09:21:07 chafik Exp $
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
#include "nel/ai/nl_ai.h"
#include "nel/ai/agent/volatil_memory.h"
#include "nel/ai/agent/agent_digital.h"
#include "nel/ai/agent/msg_on_change.h"
namespace NLAIAGENT
{
	const NLAIC::CIdentType *CHashTimerManager::IdHashTimerManager = NULL;
	sint CHashTimerManager::MaxSubscribByTimer = 50;
	uint CHashTimerManager::Time = 1000;

	CHashTimerManager::CHashTimerManager(const CHashTimerManager &)
	{
	}

	CAgentWatchTimer *CHashTimerManager::getTimer()
	{
		std::list<CAgentWatchTimer *>::const_iterator i = TimerAccount.begin();
		while( i != TimerAccount.end())
		{
			CAgentWatchTimer *c = *i ++;
			if(c->getSubscribedCount() < CHashTimerManager::MaxSubscribByTimer) return c;
		}

		TimerAccount.push_back(new CAgentClockTimer());
		CAgentWatchTimer *x = TimerAccount.back();
		x->setClock(CHashTimerManager::Time + (rand() % (CHashTimerManager::Time/4)));
		x->attach();
		return TimerAccount.back();
	}

	void CHashTimerManager::initClass()
	{
		CHashTimerManager h;
		CHashTimerManager::IdHashTimerManager = new NLAIC::CIdentType (	"HashTimerManager", 
																		NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)h),
																		NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),
																		NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));
	}

	void CHashTimerManager::releaseClass()
	{
		delete CHashTimerManager::IdHashTimerManager;
	}
	
	CHashTimerManager *CVolatilMemmory::Timer = NULL;
	const NLAIC::CIdentType *CVolatilMemmory::IdVolatilMemmory = NULL;

	CVolatilMemmory::CVolatilMemmory(IBaseGroupType *l):  _Time (CHashTimerManager::Time ),_List(l),_PairToDelete (NULL)
	{		
	}	

	CVolatilMemmory::CVolatilMemmory(const CVolatilMemmory &v): _Time (CHashTimerManager::Time ), _List((IBaseGroupType *)v._List->clone()),_PairToDelete (NULL)
	{
		_List->release();
	}

	CVolatilMemmory::~CVolatilMemmory()
	{
		_List->release();
	}
	
	void CVolatilMemmory::init()
	{
		static NLAIC::CIdentType idMsgVolatilMemmoryTimer ("MsgVolatilMemmoryTimer");		
		CAgentWatchTimer *t = Timer->getTimer();
		t->addAttrib(this, (IMessageBase*)idMsgVolatilMemmoryTimer.allocClass());
	}

	void CVolatilMemmory::pushFront(uint time,IObjectIA *p)
	{
		CPairType *pr = new CPairType(new IntegerType((sint32)(time + _Time)), p);
		testIfBest(pr);
		_List->pushFront(pr);
	}

	void CVolatilMemmory::pushBack(uint time,IObjectIA *p)
	{		
		CPairType *pr = new CPairType(new IntegerType((sint32)(time + _Time)), p);
		testIfBest(pr);
		_List->push( pr );
	}

	void CVolatilMemmory::popFront()
	{
		_List->popFront();
		if(_List->getFront() == _PairToDelete) searchBest();
	}
	void CVolatilMemmory::popBack()
	{
		_List->pop();
		if(_List->get() == _PairToDelete) searchBest();
	}

	IObjectIA *CVolatilMemmory::Front() const
	{
		return ((const CPairType *)_List->getFront())->second();
	}

	IObjectIA *CVolatilMemmory::Back() const
	{
		return ((const CPairType *)_List->get())->second();
	}

	void CVolatilMemmory::testIfBest(const CPairType *p)
	{
		if(_PairToDelete == NULL)
		{
			_PairToDelete = p;
			return;
		}
		IntegerType *n = (IntegerType *)p->first();
		IntegerType *a = (IntegerType *)_PairToDelete->first();
		IObjetOp *x = *a < *n;
		if( !x->isTrue () ) _PairToDelete = p;
		x->release();
	}

	void CVolatilMemmory::searchBest()
	{
		IntegerType maximum(_Time + 360000);
		IntegerType *u = &maximum;
		CConstIteratorContener r = _List->getConstIterator();
		while(!r.isInEnd())
		{
			const CPairType *p = (const CPairType *)((const IObjetOp*)r++);
			IntegerType *n = (IntegerType *)p->first();
			IObjetOp *x = *u > *n;
			if(_PairToDelete == NULL || x->isTrue ())
			{
				n = u;
				_PairToDelete = p;
			}
			x->release();
		}
	}

	void CVolatilMemmory::erase(CConstIteratorContener &i)
	{
		CIteratorContener it(i);
		it.erase();
		if((const IObjetOp*)i == _PairToDelete) searchBest();		
	}

	void CVolatilMemmory::sendUpdateMessage(IObjectIA *o)
	{
		std::list<IObjectIA *>::iterator i = _Connecter.begin();

		while(i != _Connecter.end())
		{
			IMessageBase *msg = new COnChangeMsg;
			NLAIAGENT::CMessageGroup group(1);
			msg->setGroup(group);
			msg->setSender(this);
			msg->setReceiver(*i);
			o->incRef();
			msg->push(o);
			(*i)->sendMessage(msg);
		}
	}

	void CVolatilMemmory::addAccount(IObjectIA *a)
	{		
		_Connecter.push_back(a);
	}

	void CVolatilMemmory::releaseAccount(IObjectIA *a)
	{
		std::list<IObjectIA *>::iterator i = _Connecter.begin();
		while(i != _Connecter.end())
		{			
			if( (*i) == a)
			{
				_Connecter.erase(i);
				break;
			}

			i++;
		}		
	}
	IObjectIA::CProcessResult CVolatilMemmory::sendMessage(IMessageBase *msg)
	{
		static NLAIC::CIdentType idMsgVolatilMemmoryTimer ("MsgVolatilMemmoryTimer");

		if(msg->getType() == idMsgVolatilMemmoryTimer)
		{
			runMessage((CAgentWatchTimer *)msg->getSender());
		}
		else
		{
			msg->release();
		}

		return IObjectIA::CProcessResult();
	}

	void CVolatilMemmory::runMessage(CAgentWatchTimer *timer)
	{
		if(_PairToDelete == NULL) return;
		_Time += timer->getClock();
		const IntegerType &u = (const IntegerType &)*_PairToDelete->first();
		uint t = (uint)u.getValue();

		IntegerType maximum(_Time + 360000);
		IntegerType *max = &maximum;

		const CPairType *oldPair = _PairToDelete, *newPair = NULL;

		if(_Time >= t)
		{
			CIteratorContener it = _List->getIterator();
			while(!it.isInEnd())
			{
				CPairType *p = (CPairType *)((const IObjetOp*)it);
				if(p == _PairToDelete)
				{
					it.erase();
					p->release();
					oldPair = NULL;
					_PairToDelete = NULL;
				}
				else				
				{
					IntegerType *u = (IntegerType *)p->first();
#ifdef NL_DEBUG
					sint32 d = u->getValue();
#endif
					(*u) -= IntegerType((sint32) timer->getClock());
#ifdef NL_DEBUG
					d = u->getValue();
#endif
					if(u->getValue() <= 0 || _Time >= (uint) u->getValue())
					{
						it.erase();
						p->release();
					}
					else
					{
						IObjetOp *x = *u < *max;
						if(x->isTrue ())
						{
							max = u;
							newPair = p;							
						}
						it ++;
						x->release();
					}
				}				
			}
			_PairToDelete = newPair;
		}

	}

	void CVolatilMemmory::onKill(IConnectIA *a)
	{
		releaseAccount(a);
	}

	void CVolatilMemmory::getDebugString(std::string &t) const
	{
		CConstIteratorContener r = _List->getConstIterator();

		while(r.isInEnd())
		{
			std::string s;
			(r ++)->getDebugString(s);
			t += s;
		}
	}

	void CVolatilMemmory::initClass()
	{

		CVolatilMemmory::Timer = new CHashTimerManager();
		CVolatilMemmory h;
		CVolatilMemmory::IdVolatilMemmory = new NLAIC::CIdentType (	"VolatilMemmory", 
																	NLAIC::CSelfClassFactory((const NLAIC::IBasicInterface &)h), 
																	NLAIC::CTypeOfObject(NLAIC::CTypeOfObject::tObject),
																	NLAIC::CTypeOfOperator(NLAIC::CTypeOfOperator::opNone));
		std::string msgStr;
		std::string scriptName("MsgVolatilMemmoryTimer");
		msgStr = std::string("From Message : Define MsgVolatilMemmoryTimer\n{");
		msgStr += std::string("Component:\n");		
		msgStr += std::string("End\n}\n");		
		NLAILINK::buildScript(msgStr,scriptName);
	}

	void CVolatilMemmory::releaseClass()
	{
		delete CVolatilMemmory::IdVolatilMemmory;
		delete CVolatilMemmory::Timer;
	}
}
