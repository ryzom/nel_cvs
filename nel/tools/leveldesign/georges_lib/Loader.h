// Loader.h: interface for the CLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOADER_H__3A379212_CAE2_48A7_84FA_3EB5D45DC7A9__INCLUDED_)
#define AFX_LOADER_H__3A379212_CAE2_48A7_84FA_3EB5D45DC7A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FormLoader.h"
#include "MoldLoader.h"

class CLoader  
{
	CFormLoader fl;
	CMoldLoader ml;

public:
	CLoader();
	virtual ~CLoader();

	void LoadForm( CForm& _f, const CStringEx& _sxfilename )
	{
		fl.LoadForm( _f, _sxfilename );
	}

	void LoadForm( CForm& _f, const CStringEx& _sxfilename, const CStringEx& _sxdate ) 
	{
		fl.LoadForm( _f, _sxfilename, _sxdate );
	}

	void SaveForm( CForm& _f, const CStringEx& _sxfilename )
	{
		fl.SaveForm( _f, _sxfilename );
	}
	
	CMoldElt* LoadMold( const CStringEx _sxfilename )
	{
		return( ml.LoadMold( _sxfilename ) );
	}

	CMoldElt* LoadMold( const CStringEx _sxfilename, const CStringEx _sxdate ) 
	{
		return( ml.LoadMold( _sxfilename, _sxdate ) );
	}
};

#endif // !defined(AFX_LOADER_H__3A379212_CAE2_48A7_84FA_3EB5D45DC7A9__INCLUDED_)
