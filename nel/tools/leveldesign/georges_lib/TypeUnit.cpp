// TypeUnit.cpp: implementation of the CTypeUnit class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TypeUnit.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTypeUnit::CTypeUnit( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv, const CStringEx _sxf )
{
	sxlowlimit = _sxll;
	sxhighlimit = _sxhl;
	sxdefaultvalue = _sxdv;
	sxformula = _sxf;
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
	return( _sxbasevalue );
}

