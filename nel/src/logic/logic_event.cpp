/** \file logic_event.cpp
 * 
 *
 * $Id: logic_event.cpp,v 1.2 2002/03/25 16:20:13 lecroart Exp $
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


#include "nel/logic/logic_event.h"
#include "nel/logic/logic_state_machine.h"

#include "nel/net/service.h"

using namespace NLMISC;
using namespace NLNET;
using namespace std;

namespace NLLOGIC
{

//----------------------------------- MESSAGE ----------------------------------


//-------------------------------------------------
// serial
//
//-------------------------------------------------
void CLogicEventMessage::serial( IStream &f )
{
	f.xmlPush("EVENT_MESSAGE");

	f.serial( Destination );
	f.serial( DestinationId );
	f.serial( MessageId );
	f.serial( Arguments );

	f.xmlPop();

} // serial //







//----------------------------------- ACTION ----------------------------------


//-------------------------------------------------
// enableSendMessage
//
//-------------------------------------------------
void CLogicEventAction::enableSendMessage()
{
	EventMessage.ToSend = true;

} // enableSendMessage //


//-------------------------------------------------
// serial
//
//-------------------------------------------------
void CLogicEventAction::serial( IStream &f )
{
	f.xmlPush("EVENT_ACTION");

	f.serial( IsStateChange );
	if( IsStateChange )
	{
		f.serial( StateChange );
	}
	else
	{
		f.serial( EventMessage );
	}

	f.xmlPop();

} // serial //









//----------------------------------- EVENT ----------------------------------



//-------------------------------------------------
// reset
//
//-------------------------------------------------
void CLogicEvent::reset()
{ 
	EventAction.EventMessage.Sent = false;
	EventAction.EventMessage.ToSend = false;

} // reset //



//-------------------------------------------------
// setLogicStateMachine
//
//-------------------------------------------------
void CLogicEvent::setLogicStateMachine( CLogicStateMachine * logicStateMachine )
{ 
	if( logicStateMachine == 0 )
	{
		nlwarning("(LOGIC)<CLogicEvent::setLogicStateMachine> The state machine is null");
	}
	else
	{
		// init the logic state machine for this event
		_LogicStateMachine = logicStateMachine;
	}

} // setLogicStateMachine //



//-------------------------------------------------
// testCondition
//
//-------------------------------------------------
bool CLogicEvent::testCondition()
{
	if( _LogicStateMachine )
	{
		if( ConditionName != "no_condition" )
		{
			CLogicCondition cond;
			if( _LogicStateMachine->getCondition( ConditionName, cond ) )
			{
				return cond.testLogic();
			}
			else
			{
				nlwarning("(LOGIC)<CLogicEvent::testCondition> Condition %s not found in the state machine",ConditionName.c_str());	
				return false;
			}
		}
		else
		{
			nlwarning("(LOGIC)<CLogicEvent::testCondition> Condition undefined");	
			return false;
		}
	}
	else
	{
		nlwarning("(LOGIC)<CLogicEvent::testCondition> The state machine managing this event is Null");
	}
	
	return false;	

} // testCondition //


//-------------------------------------------------
// serial
//
//-------------------------------------------------
void CLogicEvent::serial( IStream &f )
{
	f.xmlPush("EVENT");
	
	f.serial( ConditionName );
	f.serial( EventAction );
	
	f.xmlPop();

} // serial //

} // NLLOGIC


