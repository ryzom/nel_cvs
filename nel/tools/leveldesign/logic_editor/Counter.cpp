// Counter.cpp: implementation of the CCounter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "logic_editor.h"
#include "Counter.h"

#include "logic/logic_variable.h"

#include <string>

using namespace std;

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCounter::CCounter(const CString &name)
{
	m_sName = name;
	m_sMode = "loop";
	m_sWay = "up";

	m_nUpperLimit = 0;
	m_nLowerLimit = 0;
}

CCounter::~CCounter()
{

}

//-----------------------------------------------------
//	cCounterToCLogicCounter
//
//-----------------------------------------------------
void cCounterToCLogicCounter( CCounter& counter, CLogicCounter& logicCounter )
{
	// counter name
	logicCounter.setName( string((LPCSTR)counter.m_sName) );
	
	// running mode
	if( counter.m_sMode == "shuttle" )
		logicCounter.Mode.setValue( CLogicCounter::SHUTTLE );
	if( counter.m_sMode == "loop" )
		logicCounter.Mode.setValue( CLogicCounter::LOOP );
	if( counter.m_sMode == "stop_on_arrival" )
		logicCounter.Mode.setValue( CLogicCounter::STOP_AT_LIMIT );
	
	// running state
	// TODO

	/// lower limit for counter
	logicCounter.LowLimit.setValue( counter.m_nLowerLimit );

	/// higher limit for counter
	logicCounter.HighLimit.setValue( counter.m_nUpperLimit );
	
	// TODO : phase, period,...

} // cCounterToCLogicCounter //


//-----------------------------------------------------
//	cLogicCounterToCCounter
//
//-----------------------------------------------------
void cLogicCounterToCCounter( CLogicCounter& logicCounter, CCounter& counter )
{
	// counter name
	counter.m_sName = CString( logicCounter.getName().c_str() );

	// running mode
	switch( logicCounter.Mode.getValue() )
	{
		case CLogicCounter::SHUTTLE			: counter.m_sMode = "shuttle";			break;
		case CLogicCounter::LOOP			: counter.m_sMode = "loop";				break;
		case CLogicCounter::STOP_AT_LIMIT	: counter.m_sMode = "stop_on_arrival";	break;
	}

	// lower limit for counter
	counter.m_nLowerLimit = (long)logicCounter.LowLimit.getValue();

	// higher limit for counter
	counter.m_nUpperLimit = (long)logicCounter.HighLimit.getValue();



} // cLogicCounterToCCounter //