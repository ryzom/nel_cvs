// TypeUnitIntUnsigned.cpp: implementation of the CTypeUnitIntUnsigned class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TypeUnitIntUnsigned.h"
#include <math.h>

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

/*
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
*/

CStringEx CTypeUnitIntUnsigned::Format( const CStringEx _sxvalue ) const
{
	if( _sxvalue.empty() )
		return( sxdefaultvalue );

	std::vector< std::pair< CStringEx, CStringEx > > modificationValues;
	CStringEx value( _sxvalue );
	value.purge();
	while( value[0] == '<' )
	{
		unsigned int pos = value.find( '>' );
		if( pos == -1 )
			break;
		CStringEx sxoperateur = value.get_mid( 1, 1 );
		CStringEx sxoperande = value.get_mid( 2, pos-2);
		value.right( value.size()-pos-1 );
		modificationValues.push_back( std::make_pair( sxoperateur, sxoperande ) );
	}
	if( modificationValues.size() )
	{
		CStringEx sxr;
		for( std::vector< std::pair< CStringEx, CStringEx > >::iterator it = modificationValues.begin(); it != modificationValues.end(); ++it )
		{
			sxr += CStringEx( "<" );
			sxr += it->first;
			sxr += it->second;
			sxr += CStringEx( ">" );
		}
		return( sxr );
	}
	else
	{
		unsigned __int64 ivalue = _atoi64( _sxvalue.c_str() );
		if( ivalue < ilowlimit )
			ivalue = ilowlimit;
		if( ivalue > ihighlimit )
			ivalue = ihighlimit;

		char pc[256];
		_ui64toa( ivalue, pc, 10 );
		return( CStringEx( pc ) );
	}
}

/*									
CStringEx CTypeUnitIntUnsigned::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	nlassert( !_sxbasevalue.empty() );
	if( _sxvalue.empty() )
		return( _sxbasevalue );
	return( Format( _sxvalue ) );
}
*/

CStringEx CTypeUnitIntUnsigned::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	nlassert( !_sxbasevalue.empty() );
	if( _sxvalue.empty() )
		return( _sxbasevalue );

	std::vector< std::pair< CStringEx, CStringEx > > modificationValues;
	CStringEx value( _sxvalue );
	value.purge();
	while( value[0] == '<' )
	{
		unsigned int pos = value.find( '>' );
		if( pos == -1 )
			break;
		CStringEx sxoperateur = value.get_mid( 1, 1 );
		CStringEx sxoperande = value.get_mid( 2, pos-2);
		value.right( value.size()-pos-1 );
		modificationValues.push_back( std::make_pair( sxoperateur, sxoperande ) );
	}
	if( modificationValues.size() )
	{
		signed __int64 ir = _atoi64( _sxbasevalue.c_str() );
		for( std::vector< std::pair< CStringEx, CStringEx > >::iterator it = modificationValues.begin(); it != modificationValues.end(); ++it )
		{
			signed __int64 ivalue = _atoi64( it->second.c_str() );
			if( it->first == "+" )
				ir += ivalue;
			else if( it->first == "*" )
					ir *= ivalue;
				else if( it->first == "-" )
						ir -= ivalue;
					else if( it->first == "/" )
							ir /= ivalue;   
						else if( it->first == "^" )
						{
							ir = (__int64)( pow( (double)(ir), (double)(ivalue) ) );   
						}
		}
		unsigned __int64 uir = ir;
		if( uir < ilowlimit )
			uir = ilowlimit;
		if( uir > ihighlimit )
			uir = ihighlimit;
		char pc[256];
		_ui64toa( uir, pc, 10 );
		return( CStringEx( pc ) );
	}
	else
	{
		unsigned __int64 ivalue = _atoi64( _sxvalue.c_str() );
		if( ivalue < ilowlimit )
			ivalue = ilowlimit;
		if( ivalue > ihighlimit )
			ivalue = ihighlimit;
		char pc[256];
		_i64toa( ivalue, pc, 10 );
		return( CStringEx( pc ) );
	}
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

