// Item.cpp: implementation of the CItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Item.h"
#include "Loader.h"
#include "MoldEltDefine.h"
#include "MoldEltType.h"
#include "ItemEltAtom.h"
#include "ItemEltStruct.h"
#include "ItemEltList.h"
#include "common.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItem::	CItem()
{
	pitemes = 0;
	pitemelparents = 0;      
	pitemeacomments = 0;
}

CItem::~CItem()
{
	Clear();
}

void CItem::Clear()
{
	if( pitemes )
		delete( pitemes );
	pitemes = 0;
	if( pitemelparents )
		delete( pitemelparents );
	pitemes = 0;
	if( pitemeacomments )
		delete( pitemeacomments );
	pitemes = 0;
}

void CItem::SetLoader( CLoader* const _pl )
{
	nlassert( _pl );
	pl = _pl;
}

void CItem::Load( const CStringEx& _sxfullname )
{
	Clear();

	// Load the form
	CForm form, formcurrent, formparent;
	pl->LoadForm( formcurrent, _sxfullname );
	
	// Load Parents
	unsigned int i = 0;
	CStringEx sxparent = formcurrent.GetParent( 0 );
	while( !sxparent.empty() )								
	{
		pl->LoadSearchForm( form, sxparent );			
		formparent += form;
		i++;
		sxparent = formcurrent.GetParent( i );
	}

	// Find the name of the dfn file
	unsigned int ipos = _sxfullname.reverse_find('.');
	if( ipos == -1 )
		return;
	CStringEx moldfilename = _sxfullname.get_right(_sxfullname.length()-ipos-1);
	moldfilename += ".dfn";

	// Load the mold and build the item's tree
	CMoldElt* pme = pl->LoadMold( moldfilename );
	CMoldEltDefine* pmed = dynamic_cast< CMoldEltDefine* >( pme );
	nlassert( pmed );
	pitemes = new CItemEltStruct( pl );
	pitemes->BuildItem( pmed );

	// Fill the tree with parents' form fields 
	pitemes->FillParent( formparent.GetBody() );

	// Fill the tree with current's form
	pitemes->FillCurrent( formcurrent.GetBody() );
}

void CItem::New( const CStringEx& _sxdfnfilename )
{
	Clear();
	CMoldElt* pme = pl->LoadMold( _sxdfnfilename );
	CMoldEltDefine* pmed = dynamic_cast< CMoldEltDefine* >( pme );
	nlassert( pmed );
	pitemes = new CItemEltStruct( pl );
	pitemes->BuildItem( pmed );
}

void CItem::Load( const CStringEx& _sxfilename, const CStringEx _sxdate ) 
{
}

void CItem::Save( const CStringEx& _sxfilename )
{
	if( !pitemes )
		return;
	CForm form;
	pitemes->BuildForm( form.GetBody() );
	pl->SaveForm( form, _sxfilename );
}

CItemElt* CItem::GetElt( const unsigned int _index ) const
{
	if( pitemes )
		return( pitemes->GetElt( _index ) );
	return( 0 );
}

CItemElt* CItem::GetElt( const CStringEx _sxname ) const
{
	if( pitemes )
		return( pitemes->GetElt( _sxname ) );
	return( 0 );
}


void CItem::SetCurrentValue( const unsigned int _index, const CStringEx s )
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return;
	pie->SetCurrentValue( s );
}

unsigned int CItem::GetNbElt() const
{
	if( pitemes )
		return( pitemes->GetNbElt() );
	return( 0 );
}

unsigned int CItem::GetNbElt( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return( 0 );
	return( pie->GetNbElt() );
}

unsigned int CItem::GetInfos( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return( 0 );
	return( pie->GetInfos() );
}

CStringEx CItem::GetName( const unsigned int _index ) const
{

	CItemElt* pie = GetElt( _index );
	if( !pie )
		return( CStringEx() );
	return( pie->GetName() );
}

CStringEx CItem::GetCurrentResult( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return( CStringEx() );
	return( pie->GetCurrentResult() );
}

CStringEx CItem::GetCurrentValue( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return( CStringEx() );
	return( pie->GetCurrentValue() );
}

CStringEx CItem::GetFormula( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return( CStringEx() );
	return( pie->GetFormula() );
}

bool CItem::IsEnum( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return( false );
	return( ( pie->GetInfos() & ITEM_ISENUM ) != 0 );
}

void CItem::GetListPredef( const unsigned int _index, std::vector< CStringEx >& _vsx ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return;
	CItemEltAtom* piea = dynamic_cast< CItemEltAtom* >( pie );
	if( !piea )
		return;

	CMoldEltType* pmet = piea->GetMoldType();
	
	unsigned int i = 0;
	CStringEx sx = pmet->GetPredefDesignation( i++ );
	while( !sx.empty() )
	{
		_vsx.push_back( sx );
		sx = pmet->GetPredefDesignation( i++ );
	}
}

void CItem::AddListParent( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return;
	CItemEltList* piel = dynamic_cast< CItemEltList* >( pie );
	if( !piel )
		return;
	piel->NewElt();
}

void CItem::AddListChild( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return;
	CItemElt* piep = pie->GetListParent();
	if( !piep )
		return;
	CItemEltList* piel = dynamic_cast< CItemEltList* >( piep );
	if( !piel )
		return;
	piel->AddElt( pie );
}

void CItem::DelListChild( const unsigned int _index ) const
{
	CItemElt* pie = GetElt( _index );
	if( !pie )
		return;
	CItemElt* piep = pie->GetListParent();
	if( !piep )
		return;
	CItemEltList* piel = dynamic_cast< CItemEltList* >( piep );
	if( !piel )
		return;
	piel->DelElt( pie );
}
