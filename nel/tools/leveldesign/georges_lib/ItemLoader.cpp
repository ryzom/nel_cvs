// ItemLoader.cpp: implementation of the CItemLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ItemLoader.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CItemLoader::CItemLoader()
{
}

CItemLoader::~CItemLoader()
{
}

void CItemLoader::LoadItem( CItem& _item, const CStringEx& _sxfilename )
{
	_item.Load( _sxfilename );
}

void CItemLoader::LoadItem( CItem& _item, const CStringEx& _sxfilename, const CStringEx& _sxdate ) 
{
	_item.Load( _sxfilename, _sxdate );
}

void CItemLoader::SaveItem( CItem& _item, const CStringEx& _sxfilename )
{
	_item.Save( _sxfilename );
}

