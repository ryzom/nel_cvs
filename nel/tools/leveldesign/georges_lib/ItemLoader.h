// ItemLoader.h: interface for the CItemLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ITEMLOADER_H__50EEC0FB_1F29_464D_A34D_2DCFA762563D__INCLUDED_)
#define AFX_ITEMLOADER_H__50EEC0FB_1F29_464D_A34D_2DCFA762563D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Item.h"

class CItemLoader  
{
//	std::map< CStringEx, CFormFile* >;

public:
	CItemLoader();
	virtual ~CItemLoader();

	void LoadItem( CItem& _f, const CStringEx& _sxfilename );
	void LoadItem( CItem& _f, const CStringEx& _sxfilename, const CStringEx& _sxdate ); 
	void SaveItem( CItem& _f, const CStringEx& _sxfilename );
};

#endif // !defined(AFX_ITEMLOADER_H__50EEC0FB_1F29_464D_A34D_2DCFA762563D__INCLUDED_)
