/** \file opcode_lpt.cpp
 *
 * $Id: goal_path.cpp,v 1.4 2002/08/26 13:58:11 portier Exp $
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
#include "nel/ai/logic/goal_path.h"
#include "nel/ai/agent/object_type.h"
#include "nel/ai/agent/msg_goal.h"


namespace NLAILOGIC
{
	CGoalPath::CGoalPath(NLAIAGENT::IAgentManager *manager) : CActorScript( manager, true )
	{
	}

	CGoalPath::CGoalPath(NLAIAGENT::IAgentManager *manager, 
							NLAIAGENT::IBasicAgent *father,
							   std::list<NLAIAGENT::IObjectIA *> &components,	
							   NLAISCRIPT::CAgentClass *actor_class )
	: CActorScript(manager, father, components, actor_class )
	{	
	}	


	void CGoalPath::addGoal( CGoal *goal, bool action, int on_success, int on_failure )
	{
		_Goals.push_back( goal );
		_JmpNext.push_back( action );
		_OnSucces.push_back( on_success );
		_OnFailure.push_back( on_failure );
	}

	void CGoalPath::success()
	{
		if ( _CurrentState < ( _Goals.size() - 1 ) )
		{
			_CurrentState++;
			NLAIAGENT::IMessageBase *msg = new NLAIAGENT::CGoalMsg((NLAIAGENT::IBasicAgent *)NULL);
			msg->setPerformatif(NLAIAGENT::IMessageBase::PAchieve);
			CGoal *g = _Goals[ _CurrentState ];
			g->setSender( this );
			msg->set(0, g );
			msg->setSender( this );
			msg->setReceiver( _Father);
			_Father->sendMessage(msg);

		}
		else
		{
			// Sends the launcher a succes, then dies
			CActorScript::success();
		}
	}

	void CGoalPath::failure()
	{
		if ( _CurrentState < ( _Goals.size() - 1 ) )
		{
			// Relaunches 
			NLAIAGENT::IMessageBase *msg = new NLAIAGENT::CGoalMsg((NLAIAGENT::IBasicAgent *)NULL);
			msg->setPerformatif(NLAIAGENT::IMessageBase::PAchieve);
			CGoal *g = _Goals[ _CurrentState ];
			g->setSender( this );
			msg->set(0, g );
			msg->setSender( this );
			msg->setReceiver( _Father);
			_Father->sendMessage(msg);

		}
		else
		{
			// Sends the launcher a failure, then dies
			CActorScript::failure();
		}
	}


//	sint32 getMethodIndexSize() const;
//	virtual tQueue getPrivateMember(const IVarName *,const IVarName *,const IObjectIA &) const;
//	virtual IObjectIA::CProcessResult runMethodBase(int heritance, int index,IObjectIA *);
//	virtual IObjectIA::CProcessResult runMethodBase(int index,IObjectIA *);


	NLAIAGENT::tQueue CGoalPath::getPrivateMember(const NLAIAGENT::IVarName *className,const NLAIAGENT::IVarName *funcName,const NLAIAGENT::IObjectIA &params) const
	{

#ifdef NL_DEBUG	
	std::string nameP;
	std::string nameM;
	funcName->getDebugString(nameM);
	params.getDebugString(nameP);
	const char *dbg_class_name = (const char *) getType();
#endif
		static NLAIAGENT::CStringVarName constructor_name("Constructor");
		static NLAIAGENT::CStringVarName add_goal_name("AddGoal");

		NLAIAGENT::tQueue r;
		if(className == NULL)
		{
			if( (*funcName) == constructor_name )
			{					
				NLAIAGENT::CObjectType *c = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( CGoal::IdGoal ) );					
				r.push( NLAIAGENT::CIdMethod( 0 + CActorScript::getMethodIndexSize(), 0.0, NULL, c) );					
			}

			if( (*funcName) == add_goal_name )
			{					
				NLAIAGENT::CObjectType *c = new NLAIAGENT::CObjectType( new NLAIC::CIdentType( CGoal::IdGoal ) );					
				r.push( NLAIAGENT::CIdMethod( 1 + CActorScript::getMethodIndexSize(), 0.0, NULL, c) );					
			}
		}

		if ( r.empty() )
			return CActorScript::getPrivateMember(className, funcName, params);
		else
			return r;
	}


	NLAIAGENT::IObjectIA::CProcessResult CGoalPath::runMethodBase(int heritance, int index, NLAIAGENT::IObjectIA *)
	{
		return IObjectIA::CProcessResult();
	}

	NLAIAGENT::IObjectIA::CProcessResult CGoalPath::runMethodBase(int index, NLAIAGENT::IObjectIA *p)
	{
		NLAIAGENT::IBaseGroupType *param = (NLAIAGENT::IBaseGroupType *)p;

		switch(index - CActorScript::getMethodIndexSize())
		{
			case 0:
				{					

#ifdef NL_DEBUG
					//const char *dbg_name = name->getStr().getString();
#endif
					_Father = (NLAIAGENT::CProxyAgentMail *) param->getFront();
					param->popFront();
					// If the constructor() function is explicitely called and the object has already been initialised
					while ( param->size() )
					{
						_Goals.push_back( (NLAILOGIC::CGoal *) param->getFront()->clone() );
						param->popFront();
					}
					return IObjectIA::CProcessResult();
				} 
				break;

			case 1:
				{					

#ifdef NL_DEBUG
					//const char *dbg_name = name->getStr().getString();
#endif
					// If the constructor() function is explicitely called and the object has already been initialised
					while ( param->size() )
					{
						_Goals.push_back( (NLAILOGIC::CGoal *) param->getFront()->clone() );
						param->popFront();
					}
					return IObjectIA::CProcessResult();
				} 
				break;

		}

		return CActorScript::runMethodBase( index, p );
	}

	sint32 CGoalPath::getMethodIndexSize() const
	{
		return CActorScript::getMethodIndexSize() + 2;
	}

	const NLAIC::IBasicType *CGoalPath::clone() const
	{
		NLAIC::IBasicInterface *m = new CGoalPath( *this );
		return m;
	}

	const NLAIC::IBasicType *CGoalPath::newInstance() const
	{
		return clone();
	}

	const NLAIC::CIdentType &CGoalPath::getType() const
	{
		return IdGoalPath;
	}

	int CGoalPath::getBaseMethodCount() const
	{
		return CActorScript::getBaseMethodCount() + 2;
	}
/*
	void CGoalPath::onPause()
	{
		if ( _Father != NULL )
		{
			NLAIAGENT::IMessageBase *msg = new NLAIAGENT::CCancelGoalMsg((NLAIAGENT::IBasicAgent *)NULL);
			msg->setPerformatif(NLAIAGENT::IMessageBase::PAchieve);
			msg->set(0, _Goals[ _CurrentState ] );
			msg->setSender( this );
			msg->setReceiver( _Father);
			_Father->sendMessage(msg);
		}
	}

	void CGoalPath::onRestart()
	{
		if ( _Father != NULL )
		{
			NLAIAGENT::IMessageBase *msg = new NLAIAGENT::CGoalMsg((NLAIAGENT::IBasicAgent *)NULL);
			msg->setPerformatif(NLAIAGENT::IMessageBase::PAchieve);
			msg->set(0, _Goals[ _CurrentState ] );
			msg->setSender( this );
			msg->setReceiver( _Father);
			_Father->sendMessage(msg);
		}
	}
*/

	void CGoalPath::onActivate()
	{
		_CurrentState = 0;
		if ( _Father != NULL )
		{
			NLAIAGENT::IMessageBase *msg = new NLAIAGENT::CGoalMsg((NLAIAGENT::IBasicAgent *)NULL);
			msg->setPerformatif(NLAIAGENT::IMessageBase::PAchieve);
			CGoal *g = _Goals[ _CurrentState ];
			g->setSender( this );
			msg->set(0, g );
			msg->setSender( this );
			msg->setReceiver( _Father);
			_Father->sendMessage(msg);
		}
	}

	void CGoalPath::onUnActivate()
	{
		_CurrentState = 0;
		if ( _Father != NULL )
		{
			NLAIAGENT::IMessageBase *msg = new NLAIAGENT::CCancelGoalMsg((NLAIAGENT::IBasicAgent *)NULL);
			msg->setPerformatif(NLAIAGENT::IMessageBase::PAchieve);
			msg->set(0, _Goals[ _CurrentState ] );
			msg->setSender( this );
			msg->setReceiver( _Father);
			_Father->sendMessage(msg);
		}
	}


} // NLAILOGIC
