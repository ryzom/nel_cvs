// ItemLoader.cpp: implementation of the CItemLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "Georges.h"
#include "ItemLoader.h"

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif*/

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

