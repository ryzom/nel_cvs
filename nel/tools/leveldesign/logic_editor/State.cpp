// State.cpp: implementation of the CState class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "logic_editor.h"
#include "State.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif



//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

const CString & CEvent::getEventAsString() const
{
	eventString.Empty();

	eventString = m_sConditionName;

	if (m_bActionIsMessage)
	{
		eventString += " MSG ";
		eventString += this->m_sMessageID;

	}
	else
	{
		eventString += " State Chg to ";
		eventString += m_sStateChange;
	}

	return eventString;
}






operator==( const CEvent &ev1, const CEvent &ev2)
{
	return ( (ev1.m_sConditionName == ev2.m_sConditionName) 
			&& (ev1.m_bActionIsMessage == ev2.m_bActionIsMessage )
			&& (ev1.m_sArguments == ev2.m_sArguments )
			&& (ev1.m_sMessageDestination == ev2.m_sMessageDestination)
			&& (ev1.m_sMessageID == ev2.m_sMessageID)
			&& (ev1.m_sStateChange == ev2.m_sStateChange)
		   );
}

//////////////////////////////////////////////////////////////////////
//  CState Construction/Destruction
//////////////////////////////////////////////////////////////////////


CState::CState(const CString &name )
: m_sName( name )
{
}



CState::CState( const CState &state )
{
	this->m_sName = state.m_sName;

	// copy events (allocate new objects)
	CEvent	*pEvent;
	CEvent	*newEvent;
	POSITION pos = state.m_evEvents.GetHeadPosition();
	while (pos != NULL)
	{
		pEvent = state.m_evEvents.GetNext( pos );
		if ( pEvent != NULL)
		{
			newEvent = new CEvent( *pEvent );
			this->m_evEvents.AddTail( newEvent );
		}
	}
}




CState::~CState()
{
	// delete all events
	POSITION pos = m_evEvents.GetHeadPosition();
	while (pos != NULL)
	{
		CEvent *ev = m_evEvents.GetNext( pos );
		if ( ev != NULL)
		{
			delete ev;
			ev = NULL;
		}
	}
}


BOOL CState::removeEvent( CEvent *event)
{
	// go through all the events and remove the first event equal to the one in param
	POSITION pos = m_evEvents.GetHeadPosition();
	POSITION oldpos;
	
	while (pos != NULL)
	{
		oldpos = pos;
		CEvent *ev = m_evEvents.GetNext( pos );
		if (*ev == *event)
		{
			m_evEvents.RemoveAt(oldpos);
			return TRUE;
		}
	}

	return FALSE;
}
