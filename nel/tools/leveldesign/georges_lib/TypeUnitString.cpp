// TypeUnitString.cpp: implementation of the CTypeUnitString class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TypeUnitString.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTypeUnitString::CTypeUnitString( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv, const CStringEx _sxf ) : CTypeUnit( _sxhl, _sxll, _sxdv, _sxf )
{
	if( sxhighlimit.empty() )
	{
		usihighlimit = 0xFFFF;
		sxformula = "string";
	}
	else
	{
		usihighlimit = atoi( sxhighlimit.c_str() ); 
		sxformula.format( "string[%d]", usihighlimit );
	}
}

CTypeUnitString::~CTypeUnitString()
{
}

CStringEx CTypeUnitString::Format( const CStringEx _sxvalue ) const
{
	if( _sxvalue.empty() )
		return( sxdefaultvalue );

	if( _sxvalue.length() > usihighlimit )
		return( _sxvalue.get_left( usihighlimit ) );
	else
		return( _sxvalue );
}
									
CStringEx CTypeUnitString::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	if( _sxvalue.empty() )
		return( Format( _sxbasevalue ) );
	return( Format( _sxvalue ) );
}
