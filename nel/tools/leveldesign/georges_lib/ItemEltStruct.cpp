// ItemEltStruct.cpp: implementation of the CItemEltStruct class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemEltAtom.h"
#include "ItemEltList.h"
#include "ItemEltStruct.h"
#include "MoldEltDefine.h"
#include "MoldEltDefineList.h"
#include "FormBodyElt.h"
#include "FormBodyEltStruct.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemEltStruct::CItemEltStruct( CLoader* const _pl ) : CItemElt( _pl )
{
	infos = ITEM_ISSTRUCT;
	pmed = 0;
}

CItemEltStruct::~CItemEltStruct()
{
	Clear();
}

void CItemEltStruct::Clear()
{
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
		if( *it )
			delete( *it );
	vpie.clear();
}

void CItemEltStruct::BuildItem( CMoldElt* const _pme )
{
	if( _pme->IsList() )
		pmed = dynamic_cast< CMoldEltDefineList* >( _pme );      
	else
		pmed = dynamic_cast< CMoldEltDefine* >( _pme );      
	nlassert( pmed );

	int i = 0;
	CMoldElt* pme = pmed->GetEltPtr( i );
	while( pme )
	{
		switch( pme->GetType() )
		{
		case 0:
			{
				CItemEltAtom* piea = new CItemEltAtom( pl );
				piea->BuildItem( pme );
				if( pme->IsList() )
				{
					CItemEltList* piel = new CItemEltList( pl );
					piel->BuildItem( piea );
					piel->SetName( pmed->GetEltName( i ) );
					vpie.push_back( piel );
				}
				else
				{
					piea->SetName( pmed->GetEltName( i ) );
					vpie.push_back( piea );
				}
				break;
			}
		case 1:
			{
				CItemEltStruct* pies = new CItemEltStruct( pl );
				pies->BuildItem( pme );
				if( pme->IsList() )
				{
					CItemEltList* piel = new CItemEltList( pl );
					piel->BuildItem( pies );
					piel->SetName( pmed->GetEltName( i ) );
					vpie.push_back( piel );
				}
				else
				{
					pies->SetName( pmed->GetEltName( i ) );
					vpie.push_back( pies );
				}
				break;
			}
		}
		pme = pmed->GetEltPtr( ++i );
	}
}

CStringEx CItemEltStruct::GetFormula() const 
{
	nlassert( pmed );
	return( pmed->GetFormula() );
}

void CItemEltStruct::FillParent( const CFormBodyElt* const _pfbe )
{
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
	{
		CFormBodyElt* pfbe = _pfbe->GetElt( (*it)->GetName() );
		if( pfbe )
			(*it)->FillParent( pfbe );
	}
}

void CItemEltStruct::FillCurrent( const CFormBodyElt* const _pfbe )
{
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
	{
		CFormBodyElt* pfbe = _pfbe->GetElt( (*it)->GetName() );
		if( pfbe )
			(*it)->FillCurrent( pfbe );
	}
}

CItemElt* CItemEltStruct::Clone()
{
	CItemEltStruct* pies = new CItemEltStruct( pl );
	pies->BuildItem( pmed );
	return( pies );
}

CFormBodyElt* CItemEltStruct::BuildForm()
{
	CFormBodyEltStruct* pfbes = new CFormBodyEltStruct();
	pfbes->SetName( sxname );
	BuildForm( pfbes );
	if( !pfbes->Empty() )
		return( pfbes );
	delete pfbes;
	return( 0 );
}

void CItemEltStruct::BuildForm( CFormBodyEltStruct* pfbes )
{
	for( std::vector< CItemElt* >::iterator it = vpie.begin(); it != vpie.end(); ++it )
		pfbes->AddElt( (*it)->BuildForm() );
}

unsigned int CItemEltStruct::GetNbElt() const
{
	unsigned int nb = 0;
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
		nb += (*it)->GetNbElt();
	return( ++nb );
}

CItemElt* CItemEltStruct::GetElt( const unsigned int _index ) const
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

CItemElt* CItemEltStruct::GetElt( const CStringEx _sxname ) const
{
	for( std::vector< CItemElt* >::const_iterator it = vpie.begin(); it != vpie.end(); ++it )
		if( (*it)->GetName() == _sxname )
			return( *it );
	return( 0 );
}

