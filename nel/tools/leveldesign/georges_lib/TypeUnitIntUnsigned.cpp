// TypeUnitIntUnsigned.cpp: implementation of the CTypeUnitIntUnsigned class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TypeUnitIntUnsigned.h"

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTypeUnitIntUnsigned::CTypeUnitIntUnsigned( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv ) : CTypeUnit( _sxll, _sxhl, _sxdv )
{
	ilowlimit = _atoi64( sxlowlimit.c_str() ); 
	ihighlimit = _atoi64( sxhighlimit.c_str() ); 
	idefaultvalue = _atoi64( sxdefaultvalue.c_str() ); 
	sxformula.format( "uint(%d,%d)", ilowlimit, ihighlimit );
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

	CStringEx sx;
	sx.format( "%d", ivalue );
	return( sx );
}
									
CStringEx CTypeUnitIntUnsigned::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	nlassert( !_sxbasevalue.empty() );
	if( _sxvalue.empty() )
		return( _sxbasevalue );
	
//	unsigned __int64 ibasevalue = _atoi64( _sxbasevalue.c_str() );
	unsigned __int64 ivalue = _atoi64( _sxvalue.c_str() );

	CStringEx sx;
	sx.format( "%d", ivalue );
	return( Format( sx ) );
}

//	char* pc;
//	_i64toa( ivalue, pc, 10 );
//	return( CStringEx( pc ) );
