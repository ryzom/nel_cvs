// MoldEltType.cpp: implementation of the CMoldEltType class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MoldEltTypeList.h"
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

CMoldEltTypeList::CMoldEltTypeList( CLoader* const _pl, CMoldEltType* const _pmet ) : CMoldEltType( _pl )
{
	pmet = _pmet;
	blist = true;
	benum = pmet->IsEnum();
	sxname = pmet->GetName();
}

CMoldEltTypeList::~CMoldEltTypeList()
{
}

void CMoldEltTypeList::Load( const CStringEx _sxfullname )						// TODO: Load with parents...
{
	pmet->Load( _sxfullname );
	benum = pmet->IsEnum();
	sxname = pmet->GetName();
}

void CMoldEltTypeList::Load( const CStringEx _sxfullname, const CStringEx _sxdate )
{
	pmet->Load( _sxfullname, _sxdate );
}

CStringEx CMoldEltTypeList::GetFormula()													 
{
	return( pmet->GetFormula() );
}

CStringEx CMoldEltTypeList::Format( const CStringEx _sxvalue ) const													
{
	return( pmet->Format( _sxvalue ) );
}

CStringEx CMoldEltTypeList::CalculateResult( const CStringEx _sxbasevalue, const CStringEx _sxvalue ) const	
{
	return( pmet->CalculateResult( _sxbasevalue, _sxvalue ) );
}

CStringEx CMoldEltTypeList::GetDefaultValue() const	
{
	return( pmet->GetDefaultValue() );
}

unsigned int CMoldEltTypeList::GetType() const
{
	return( pmet->GetType() );
}

CStringEx CMoldEltTypeList::GetPredefSubstitute( const CStringEx _sxdesignation ) const
{
	return( pmet->GetPredefSubstitute( _sxdesignation ) );
}

CStringEx CMoldEltTypeList::GetPredefDesignation( const CStringEx _sxsubstitute ) const
{
	return( pmet->GetPredefDesignation( _sxsubstitute ) );
}

CStringEx CMoldEltTypeList::GetPredefDesignation( const unsigned int _index ) const
{
	return( pmet->GetPredefDesignation( _index ) );
}

CMoldElt* CMoldEltTypeList::GetMold()
{
	return( pmet );
}
