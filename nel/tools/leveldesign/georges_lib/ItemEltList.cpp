// ItemEltList.cpp: implementation of the CItemEltList class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemEltList.h"
#include "ItemEltAtom.h"
#include "ItemEltStruct.h"
#include "FormBodyElt.h"
#include "FormBodyEltList.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemEltList::CItemEltList( CLoader* const _pl ) : CItemElt( _pl )
{
	piemodel = 0;
}

CItemEltList::~CItemEltList()
{
	if( piemodel )
		delete piemodel;
	piemodel = 0;
	Clear();
}

void CItemEltList::Clear()
{
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
		if( *it )
			delete( *it );
	vpie.clear();
}

void CItemEltList::BuildItem( CItemElt* const _pie )
{
	piemodel = _pie;
	nlassert( piemodel );
}

CStringEx CItemEltList::GetFormula() const 
{
	nlassert( piemodel );
	return( CStringEx("list<") + piemodel->GetFormula() + ">" );
}

void CItemEltList::FillParent( const CFormBodyElt* const _pfbe )
{
	unsigned int i = 0;
	CStringEx sx;
	CFormBodyElt* pfbe = _pfbe->GetElt(0);
	if( pfbe )
		Clear();	
	while( pfbe )
	{
		CItemElt* pie = piemodel->Clone();
		pie->FillParent( pfbe );
		sx.format( "#%d", i );
		pie->SetName( sx );
		vpie.push_back( pie );
		pfbe = _pfbe->GetElt(++i);
	}
}

void CItemEltList::FillCurrent( const CFormBodyElt* const _pfbe )
{
	unsigned int i = 0;
	CStringEx sx;
	CFormBodyElt* pfbe = _pfbe->GetElt(0);
	if( pfbe )
		Clear();	
	while( pfbe )
	{
		CItemElt* pie = piemodel->Clone();
		pie->FillCurrent( pfbe );
		sx.format( "#%d", i );
		pie->SetName( sx );
		vpie.push_back( pie );
		pfbe = _pfbe->GetElt(++i);
	}
}

CItemElt* CItemEltList::Clone()
{
	CItemEltList* piel = new CItemEltList( pl );
	piel->BuildItem( piemodel );
	return( piel );
}

CFormBodyElt* CItemEltList::BuildForm()
{
	CFormBodyEltList* pfbel = new CFormBodyEltList();
	pfbel->SetName( sxname );
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
		pfbel->AddElt( (*it)->BuildForm() );
	return( pfbel );
}

unsigned int CItemEltList::GetNbElt() const
{
	unsigned int nb = 0;
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
		nb += (*it)->GetNbElt();
	return( ++nb );
}

CItemElt* CItemEltList::GetElt( const unsigned int _index ) const
{
	if( !_index )
		return( ( CItemElt * )( this ) );
	unsigned int isum = 1;				
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
	{
		unsigned int nb = (*it)->GetNbElt();
		if( isum+nb > _index )
			return( (*it)->GetElt( _index-isum ) );
		isum += nb;
	}
	return( 0 );
}

CItemElt* CItemEltList::GetElt( const CStringEx _sxname ) const
{
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
		if( (*it)->GetName() == _sxname )
			return( *it );
	return( 0 );
}
