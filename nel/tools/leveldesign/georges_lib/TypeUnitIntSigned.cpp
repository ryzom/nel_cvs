// TypeUnitIntSigned.cpp: implementation of the CTypeUnitIntSigned class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TypeUnitIntSigned.h"

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTypeUnitIntSigned::CTypeUnitIntSigned( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv ) : CTypeUnit( _sxll, _sxhl, _sxdv )
{
	ilowlimit = _atoi64( sxlowlimit.c_str() ); 
	ihighlimit = _atoi64( sxhighlimit.c_str() ); 
	idefaultvalue = _atoi64( sxdefaultvalue.c_str() ); 
	sxformula.format( "sint(%d,%d)", ilowlimit, ihighlimit );
}

CTypeUnitIntSigned::~CTypeUnitIntSigned()
{
}

CStringEx CTypeUnitIntSigned::Format( const CStringEx _sxvalue ) const
{
	if( _sxvalue.empty() )
		return( sxdefaultvalue );

	signed __int64 ivalue = _atoi64( _sxvalue.c_str() );
	if( ivalue < ilowlimit )
		ivalue = ilowlimit;
	if( ivalue > ihighlimit )
		ivalue = ihighlimit;

	CStringEx sx;
	sx.format( "%d", ivalue );
	return( sx );
}
									
CStringEx CTypeUnitIntSigned::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	nlassert( !_sxbasevalue.empty() );
	if( _sxvalue.empty() )
		return( _sxbasevalue );
	
//	signed __int64 ibasevalue = _atoi64( _sxbasevalue.c_str() );
	signed __int64 ivalue = _atoi64( _sxvalue.c_str() );

	CStringEx sx;
	sx.format( "%d", ivalue );
	return( Format( sx ) );
}
