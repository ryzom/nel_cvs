// TypeUnitIntUnsigned.cpp: implementation of the CTypeUnitIntUnsigned class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TypeUnitIntUnsigned.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTypeUnitIntUnsigned::CTypeUnitIntUnsigned( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv, const CStringEx _sxf ) : CTypeUnit( _sxll, _sxhl, _sxdv, _sxf )
{
	ilowlimit = _atoi64( sxlowlimit.c_str() ); 
	ihighlimit = _atoi64( sxhighlimit.c_str() ); 
	idefaultvalue = _atoi64( sxdefaultvalue.c_str() ); 
	if( sxformula.empty() )
		sxformula = CStringEx( "uint(" +_sxll +"," +_sxhl +")" );
}

CTypeUnitIntUnsigned::~CTypeUnitIntUnsigned()
{
}

CStringEx CTypeUnitIntUnsigned::Format( const CStringEx _sxvalue ) const
{
	if( _sxvalue.empty() )
		return( sxdefaultvalue );

	unsigned __int64 ivalue = _atoi64( _sxvalue.c_str() );
	if( ivalue < ilowlimit )
		ivalue = ilowlimit;
	if( ivalue > ihighlimit )
		ivalue = ihighlimit;

	char pc[256];
	_ui64toa( ivalue, pc, 10 );
	return( CStringEx( pc ) );
}
									
CStringEx CTypeUnitIntUnsigned::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	nlassert( !_sxbasevalue.empty() );
	if( _sxvalue.empty() )
		return( _sxbasevalue );
	return( Format( _sxvalue ) );
}

void CTypeUnitIntUnsigned::SetDefaultValue( const CStringEx _sxdv )
{
	sxdefaultvalue = _sxdv;
	idefaultvalue = _atoi64( sxdefaultvalue.c_str() ); 
}

void CTypeUnitIntUnsigned::SetLowLimit( const CStringEx _sxll )
{
	sxlowlimit = _sxll;
	ilowlimit = _atoi64( sxlowlimit.c_str() ); 
}

void CTypeUnitIntUnsigned::SetHighLimit( const CStringEx _sxhl )
{
	sxhighlimit = _sxhl;
	ihighlimit = _atoi64( sxhighlimit.c_str() ); 
}

