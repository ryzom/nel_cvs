// Counter.cpp: implementation of the CCounter class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "logic_editor.h"
#include "Counter.h"

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
