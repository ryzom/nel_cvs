// ItemElt.cpp: implementation of the CItemElt class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemElt.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemElt::CItemElt( CLoader* const _pl )
{
	nlassert( _pl );
	pl = _pl;
	listparent = 0;
}

CItemElt::~CItemElt()
{
}

unsigned int CItemElt::GetInfos() const
{
	return( infos );
}

void CItemElt::AddInfos( const unsigned int _infos )
{
	infos |= _infos;
}

void CItemElt::SetListParent( CItemElt* const _listparent )
{
	listparent = _listparent;
}

CItemElt* CItemElt::GetListParent() const
{
	return( listparent );
}

void CItemElt::SetName( const CStringEx _sxname )
{
	sxname = _sxname;
}

CStringEx CItemElt::GetName() const
{
	return( sxname );
}

CStringEx CItemElt::GetParent() const
{
	return( sxparent );
}

CStringEx CItemElt::GetFormula() const
{
	return( CStringEx() );
}

CStringEx CItemElt::GetParentResult() const
{
	return( sxparentresult );
}

CStringEx CItemElt::GetParentValue() const
{
	return( sxparentvalue );
}

CStringEx CItemElt::GetCurrentResult() const
{
	return( sxcurrentresult );
}

CStringEx CItemElt::GetCurrentValue() const
{
	return( sxcurrentvalue );
}

void CItemElt::SetParentValue( const CStringEx _sxparentvalue )
{
}

void CItemElt::SetCurrentValue( const CStringEx _sxcurrentvalue )
{
}

void CItemElt::FillParent( const CFormBodyElt* const _pfbe )
{
}

void CItemElt::FillCurrent( const CFormBodyElt* const _pfbe )
{
}

CItemElt* CItemElt::Clone()
{
	return( 0 );
}

CFormBodyElt* CItemElt::BuildForm()
{
	return( 0 );
}

unsigned int CItemElt::GetNbElt() const
{
	return( 0 );
}

CItemElt* CItemElt::GetElt( const unsigned int _index ) const
{
	if( !_index )
		return( ( CItemElt* )( this ) );
	return( 0 );
}

CItemElt* CItemElt::GetElt( const CStringEx _sxname ) const
{
	return( 0 );
}

