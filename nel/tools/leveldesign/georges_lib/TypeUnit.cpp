// TypeUnit.cpp: implementation of the CTypeUnit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TypeUnit.h"

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTypeUnit::CTypeUnit( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv )
{
	sxlowlimit = _sxll;
	sxhighlimit = _sxhl;
	sxdefaultvalue = _sxdv;
}

CTypeUnit::~CTypeUnit()
{
}

CStringEx CTypeUnit::GetFormula() const
{
	return	sxformula;
}

CStringEx CTypeUnit::GetDefaultValue() const
{
	return	sxdefaultvalue;
}

CStringEx CTypeUnit::Format( const CStringEx _sxvalue ) const
{
	return( _sxvalue );
}
									
CStringEx CTypeUnit::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	return( _sxvalue );
}

