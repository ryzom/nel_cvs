// TypeUnitDouble.cpp: implementation of the CTypeUnitDouble class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TypeUnitDouble.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTypeUnitDouble::CTypeUnitDouble( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv, const CStringEx _sxf ) : CTypeUnit( _sxll, _sxhl, _sxdv, _sxf )
{
	dlowlimit = atof( sxlowlimit.c_str() ); 
	dhighlimit = atof( sxhighlimit.c_str() ); 
	ddefaultvalue = atof( sxdefaultvalue.c_str() ); 
	if( sxformula.empty() )
		sxformula.format( "double(%s,%s)", FormatDouble( dlowlimit ).c_str(), FormatDouble( dhighlimit ).c_str() );
}

CTypeUnitDouble::~CTypeUnitDouble()
{
}

CStringEx CTypeUnitDouble::FormatDouble( const double dvalue ) const
{
	int  decimal, sign;
	char *buffer;

	if( dvalue == 0 )
		return( CStringEx( "0.0" ));

	buffer = _fcvt( dvalue, 5, &decimal, &sign );
	CStringEx sx( buffer );
	if( decimal <= 0 )
		sx = CStringEx( "0." ) +CStringEx( '0', -decimal ) +sx;
	else
		sx.insert(decimal,".");

	while( sx[sx.length()-1] == '0' )
		sx.left( sx.length() -1 );
	
	if( sx[sx.length()-1] == '.' )
		sx += '0';
	
	if( sign )
		sx = CStringEx( "-" + sx );

	return sx;
}

CStringEx CTypeUnitDouble::Format( const CStringEx _sxvalue ) const
{
	if( _sxvalue.empty() )
		return( sxdefaultvalue );

	double dvalue = atof( _sxvalue.c_str() );
	if( dvalue < dlowlimit )
		dvalue = dlowlimit;
	if( dvalue > dhighlimit )
		dvalue = dhighlimit;
	return( FormatDouble( dvalue ) );
}
									
CStringEx CTypeUnitDouble::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	nlassert( !_sxbasevalue.empty() );
	if( _sxvalue.empty() )
		return( _sxbasevalue );
	return( Format( _sxvalue ) );
}

void CTypeUnitDouble::SetDefaultValue( const CStringEx _sxdv )
{
	sxdefaultvalue = _sxdv;
	ddefaultvalue = atof( sxdefaultvalue.c_str() ); 
}

void CTypeUnitDouble::SetLowLimit( const CStringEx _sxll )
{
	sxlowlimit = _sxll;
	dlowlimit = atof( sxlowlimit.c_str() ); 
}

void CTypeUnitDouble::SetHighLimit( const CStringEx _sxhl )
{
	sxhighlimit = _sxhl;
	dhighlimit = atof( sxhighlimit.c_str() ); 
}

