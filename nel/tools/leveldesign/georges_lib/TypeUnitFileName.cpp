// TypeUnitString.cpp: implementation of the CTypeUnitString class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TypeUnitFileName.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTypeUnitFileName::CTypeUnitFileName( const CStringEx _sxll, const CStringEx _sxhl, const CStringEx _sxdv, const CStringEx _sxf ) : CTypeUnit( _sxhl, _sxll, _sxdv, _sxf )
{
	if( sxformula.empty() )
		sxformula = "file name";
} 

CTypeUnitFileName::~CTypeUnitFileName()
{
}

CStringEx CTypeUnitFileName::Format( const CStringEx _sxvalue ) const
{
	if( _sxvalue.empty() )
		return( sxdefaultvalue );
	return( _sxvalue );
}
									
CStringEx CTypeUnitFileName::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	if( _sxvalue.empty() )
		return( Format( _sxbasevalue ) );
	return( Format( _sxvalue ) );
}
