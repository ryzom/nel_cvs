// ItemEltAtom.cpp: implementation of the CItemEltAtom class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemEltAtom.h"
#include "MoldEltType.h"
#include "FormBodyElt.h"
#include "FormBodyEltAtom.h"

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemEltAtom::CItemEltAtom( CLoader* const _pl ) : CItemElt( _pl )
{
	pmet = 0; 
}

CItemEltAtom::~CItemEltAtom()
{
}

void CItemEltAtom::BuildItem( CMoldElt* const _pme )
{
	pmet = dynamic_cast< CMoldEltType* >( _pme );      
	nlassert( pmet );
	infos = ITEM_ISATOM;
	sxparentvalue.clear();
	sxcurrentvalue.clear();
	sxoldparentvalue.clear();
	sxoldcurrentvalue.clear();
	sxparentresult = pmet->GetDefaultValue();
	sxcurrentresult = pmet->GetDefaultValue();
}

CStringEx CItemEltAtom::GetFormula() const 
{
	nlassert( pmet );
	return( pmet->GetFormula() );
}

void CItemEltAtom::SetParentValue( const CStringEx _sxparentvalue )
{
	if( _sxparentvalue.empty() )
		return;
	nlassert( pmet );
	CStringEx sxsubstitute = pmet->GetPredefSubstitute( _sxparentvalue );
	if( sxsubstitute.empty() )
	{
		sxparentvalue = pmet->Format( _sxparentvalue );
		sxparentresult = pmet->CalculateResult( pmet->GetDefaultValue(), sxparentvalue );
	}
	else
	{
		sxparentvalue = _sxparentvalue;
		sxparentresult = pmet->CalculateResult( pmet->GetDefaultValue(), sxsubstitute );
	}
}

void CItemEltAtom::SetCurrentValue( const CStringEx _sxcurrentvalue )
{
	if( _sxcurrentvalue.empty() )
	{
		sxcurrentvalue.clear();
		sxcurrentresult = sxparentresult;
		return;
	}
	nlassert( pmet );
	CStringEx sxsubstitute = pmet->GetPredefSubstitute( _sxcurrentvalue );
	if( sxsubstitute.empty() )
	{
		sxcurrentvalue = pmet->Format( _sxcurrentvalue );
		sxcurrentresult = pmet->CalculateResult( sxparentresult, _sxcurrentvalue );
	}
	else
	{
		sxcurrentvalue = _sxcurrentvalue;
		sxcurrentresult = pmet->CalculateResult( sxparentresult, sxsubstitute );
	}
}

void CItemEltAtom::FillParent( const CFormBodyElt* const _pfbe )
{
	SetParentValue( _pfbe->GetValue() );
	sxoldparentvalue = sxparentvalue;
}

void CItemEltAtom::FillCurrent( const CFormBodyElt* const _pfbe )
{
	SetCurrentValue( _pfbe->GetValue() );
	sxoldcurrentvalue = sxcurrentvalue;
}

CItemElt* CItemEltAtom::Clone()
{
	CItemEltAtom* piea = new CItemEltAtom( pl );
	piea->BuildItem( pmet );
	return( piea );
}

CFormBodyElt* CItemEltAtom::BuildForm()
{
	if( sxoldcurrentvalue.empty() && sxcurrentvalue.empty() )
		return( 0 );
	CFormBodyEltAtom* pfbea = new CFormBodyEltAtom();
	pfbea->SetName( sxname );
	pfbea->SetValue( sxcurrentvalue );
	return( pfbea );
}

unsigned int CItemEltAtom::GetNbElt() const
{
	return( 1 );
}

CItemElt* CItemEltAtom::GetElt( const unsigned int _index ) const
{
	if( !_index )
		return( ( CItemElt* )( this ) );
	return( 0 );
}


