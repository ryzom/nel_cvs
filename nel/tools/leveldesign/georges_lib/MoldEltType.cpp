// MoldEltType.cpp: implementation of the CMoldEltType class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MoldEltType.h"
#include "Loader.h"
#include "FormBodyElt.h"
#include "TypeUnitIntUnsigned.h"
#include "TypeUnitIntSigned.h"
#include "TypeUnitDouble.h"
#include "TypeUnitString.h"
#include "TypeUnitFileName.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CMoldEltType::CMoldEltType( CLoader* const _pl ) : CMoldElt( _pl )
{
}

CMoldEltType::~CMoldEltType()
{
}

void CMoldEltType::Load( const CStringEx _sxfilename )						// TODO: Load with parents...
{

////////////////////////////////////////////////////////////////////////////
// By this way, there is no parents... Does *.typ must have? NotSure....
// Be sure that the form have the fields!
// code:
	sxname = CStringEx( _sxfilename );
	CForm f;
	pl->LoadForm( f, pl->WhereIsDfnTyp( _sxfilename ) );									

	CFormBodyElt* pfbell = f.GetElt("Lowlimit");
	CFormBodyElt* pfbehl = f.GetElt("Highlimit");
	CFormBodyElt* pfbedv = f.GetElt("DefaultValue");
	CFormBodyElt* pfbeenum = f.GetElt("Enum");
	CFormBodyElt* pfbetype = f.GetElt("Type");
	CFormBodyElt* pfbeformula = f.GetElt("Formula");

	nlassert( pfbell );
	nlassert( pfbehl );
	nlassert( pfbedv );
	nlassert( pfbeenum );
	nlassert( pfbetype );
	nlassert( pfbeformula );
// To be changed quickly by:
// CItem* pitem = pl->LoadItem( _sxfilename );
// ect...
////////////////////////////////////////////////////////////////////////////

	CStringEx sxtype = pfbetype->GetValue();
	if( sxtype == "uint" )
		ptu = new CTypeUnitIntUnsigned( pfbell->GetValue(), pfbehl->GetValue(), pfbedv->GetValue(), pfbeformula->GetValue() );
	else if( sxtype == "sint" )
		ptu = new CTypeUnitIntSigned( pfbell->GetValue(), pfbehl->GetValue(), pfbedv->GetValue(), pfbeformula->GetValue() );
	else if( sxtype == "double" )
		ptu = new CTypeUnitDouble( pfbell->GetValue(), pfbehl->GetValue(), pfbedv->GetValue(), pfbeformula->GetValue() );
	else if( sxtype == "filename" )
		ptu = new CTypeUnitFileName( pfbell->GetValue(), pfbehl->GetValue(), pfbedv->GetValue(), pfbeformula->GetValue() );
	else if( sxtype == "string" )
		ptu = new CTypeUnitString( pfbell->GetValue(), pfbehl->GetValue(), pfbedv->GetValue(), pfbeformula->GetValue() );

	CFormBodyElt* pfbepredef = f.GetElt("Predef");
	if( pfbepredef )
	{
		unsigned int i = 0;
		CFormBodyElt* pfbe = pfbepredef->GetElt( i++ );
		while( pfbe )
		{
			CFormBodyElt* pfbedesignation = pfbe->GetElt( "Designation" );
			CFormBodyElt* pfbesubstitute = pfbe->GetElt( "Substitute" );
			nlassert( pfbedesignation );
			nlassert( pfbesubstitute );
			vpredef.push_back( std::make_pair( pfbedesignation->GetValue(), pfbesubstitute->GetValue() ) );
			pfbe = pfbepredef->GetElt( i++ );
		}
	}
	benum = ( pfbeenum->GetValue() == "true" );
}

void CMoldEltType::Load( const CStringEx _sxfilename, const CStringEx _sxdate )
{
}

CStringEx CMoldEltType::GetFormula()													 
{
	return( ptu->GetFormula() );
}

CStringEx CMoldEltType::Format( const CStringEx _sxvalue ) const													
{

	if( _sxvalue.empty() )
		return( _sxvalue );
	CStringEx sx = GetPredefSubstitute( _sxvalue );
	if( sx.empty() )
		if( benum )
			return( CStringEx() );
		else
			return( ptu->Format( _sxvalue ) );
	return( GetPredefDesignation( sx ) ); 
}

CStringEx CMoldEltType::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{

	if( _sxvalue.empty() )
		return( _sxbasevalue );
	CStringEx sx = GetPredefSubstitute( _sxvalue );
	if( benum )
	{
		CStringEx sx2 = ptu->CalculateResult( _sxbasevalue, sx );
		return( GetPredefDesignation( sx2 ) );
	}
	if( sx.empty() )
		return( ptu->CalculateResult( _sxbasevalue, _sxvalue ) );
	return( ptu->CalculateResult( _sxbasevalue, sx ) );
}

CStringEx CMoldEltType::GetDefaultValue() const	
{
	return( ptu->GetDefaultValue() );
}

unsigned int CMoldEltType::GetType() const
{
	return( 0 );
}

CStringEx CMoldEltType::GetPredefSubstitute( const CStringEx _sxdesignation ) const
{
	if( !_sxdesignation.empty() )
		for( std::vector< std::pair< CStringEx, CStringEx > >::const_iterator it = vpredef.begin(); it != vpredef.end(); ++it )
			if( it->first == _sxdesignation )
				return( it->second );
	return( CStringEx() );
}

CStringEx CMoldEltType::GetPredefDesignation( const CStringEx _sxsubstitute ) const
{
	for( std::vector< std::pair< CStringEx, CStringEx > >::const_iterator it = vpredef.begin(); it != vpredef.end(); ++it )
		if( it->second == _sxsubstitute )
			return( it->first );
	return( CStringEx() );
}

CStringEx CMoldEltType::GetPredefDesignation( const unsigned int _index ) const
{
	if( _index < vpredef.size() )
		return( vpredef[_index].first );
	return( CStringEx() );
}
