// Item.cpp: implementation of the CItem class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Item.h"
#include "Loader.h"
#include "MoldEltDefine.h"
#include "ItemEltAtom.h"
#include "ItemEltStruct.h"
#include "ItemEltList.h"
#include "common.h"

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif*/


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


void CItem::Load( const CStringEx& _sxfilename )
{
	// Load the form
	CForm form, formcurrent, formparent;
	pl->LoadForm( formcurrent, _sxfilename );
	
	// Load Parents
	unsigned int i = 0;
	CStringEx sxparent = formcurrent.GetParent( 0 );
	while( !sxparent.empty() )
	{
		pl->LoadForm( form, _sxfilename );
		formparent += form;
		i++;
		sxparent = formcurrent.GetParent( i );
	}

	// Find the name of the dfn file
	unsigned int ipos = _sxfilename.reverse_find('.');
	if( ipos == -1 )
		return;
	CStringEx moldfilename = _sxfilename.get_right(_sxfilename.length()-ipos-1);
	moldfilename += ".dfn";

	// Load the mold and build the item's tree
	CMoldElt* pme = pl->LoadMold( moldfilename );
	CMoldEltDefine* pmed = dynamic_cast< CMoldEltDefine* >( pme );
	nlassert( pmed );
	pitemes = new CItemEltStruct( pl );
	pitemes->BuildItem( pmed );

	// Remove Reserved Fields
//	pitemelparents = dynamic_cast< CItemEltList* >( pitemes->PruneElt( RESERVEDKEYWORD_PARENTS ) );      
//	pitemeacomments = dynamic_cast< CItemEltAtom* >( pitemes->PruneElt( RESERVEDKEYWORD_COMMENTS ) );      
	
	// Fill the tree with parents' form fields 
	pitemes->FillParent( formparent.GetBody() );

	// Fill the tree with current's form
	pitemes->FillCurrent( formcurrent.GetBody() );

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

