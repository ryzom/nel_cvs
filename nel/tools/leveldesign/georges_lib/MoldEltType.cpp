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

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif*/

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
	pl->LoadForm( f, _sxfilename );									

	CFormBodyElt* pfbell = f.GetElt("lowlimit");
	CFormBodyElt* pfbehl = f.GetElt("highlimit");
	CFormBodyElt* pfbedv = f.GetElt("defaultvalue");
	CFormBodyElt* pfbeenum = f.GetElt("enum");
	CFormBodyElt* pfbetype = f.GetElt("type");

	nlassert( pfbell );
	nlassert( pfbehl );
	nlassert( pfbedv );
	nlassert( pfbeenum );
	nlassert( pfbetype );
// To be changed quickly by:
// CItem* pitem = pl->LoadItem( _sxfilename );
// ect...
////////////////////////////////////////////////////////////////////////////

	CStringEx sxtype = pfbetype->GetValue();
	if( sxtype == "uint" )
		ptu = new CTypeUnitIntUnsigned( pfbell->GetValue(), pfbehl->GetValue(), pfbedv->GetValue() );
	else if( sxtype == "sint" )
		ptu = new CTypeUnitIntSigned( pfbell->GetValue(), pfbehl->GetValue(), pfbedv->GetValue() );
	else if( sxtype == "double" )
		ptu = new CTypeUnitDouble( pfbell->GetValue(), pfbehl->GetValue(), pfbedv->GetValue() );
	else if( sxtype == "string" )
		ptu = new CTypeUnitString( pfbell->GetValue(), pfbehl->GetValue(), pfbedv->GetValue() );

	CFormBodyElt* pfbepredef = f.GetElt("predef");
	if( pfbepredef )
	{
		unsigned int i = 0;
		CFormBodyElt* pfbe = pfbepredef->GetElt( i++ );
		while( pfbe )
		{
			CFormBodyElt* pfbedesignation = pfbe->GetElt( "designation" );
			CFormBodyElt* pfbesubstitute = pfbe->GetElt( "substitute" );
			nlassert( pfbedesignation );
			nlassert( pfbesubstitute );
			vpredef.push_back( std::make_pair( pfbedesignation->GetValue(), pfbesubstitute->GetValue() ) );
			pfbe = pfbepredef->GetElt( i++ );
		}
	}

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
	return( ptu->Format( _sxvalue ) );
}

CStringEx CMoldEltType::CalculateResult( const CStringEx _sxvalue, const CStringEx _sxbasevalue ) const	
{
	return( ptu->CalculateResult( _sxvalue, _sxbasevalue ) );
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
	for( std::vector< std::pair< CStringEx, CStringEx > >::const_iterator it = vpredef.begin(); it != vpredef.end(); ++it )
		if( it->first == _sxdesignation )
			return( it->second );
	return( CStringEx() );
}
