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
#include "io.h"

class CLoader  
{
protected:
	CFormLoader fl;
	CMoldLoader ml;
	CStringEx sxdfndirectory;
	CStringEx sxworkdirectory;
	CStringEx sxrootdirectory;

public:
	CLoader();
	virtual ~CLoader();

	void LoadForm( CForm& _f, const CStringEx& _sxfullname );
	void LoadForm( CForm& _f, const CStringEx& _sxfullname, const CStringEx& _sxdate );
	void LoadSearchForm( CForm& _f, const CStringEx& _sxfilename );
	void LoadSearchForm( CForm& _f, const CStringEx& _sxfilename, const CStringEx& _sxdate );
	void SaveForm( CForm& _f, const CStringEx& _sxfullename );
	CMoldElt* LoadMold( const CStringEx _sxfilename );
	CMoldElt* LoadMold( const CStringEx _sxfilename, const CStringEx _sxdate ); 

	CStringEx WhereIs( const CStringEx _sxdirectory, const CStringEx _sxfilename );
	CStringEx Search( const CStringEx _sxfilename);

	void SetDfnTypDirectory( const CStringEx _sxdfndirectory );
	void SetWorkDirectory( const CStringEx _sxworkdirectory );
	void SetRootDirectory( const CStringEx _sxrootdirectory );
	CStringEx GetDfnTypDirectory() const;
	CStringEx GetWorkDirectory() const;
	CStringEx GetRootDirectory() const;

	void MakeDfn( const CStringEx _sxfullname, const std::list< std::pair< CStringEx, CStringEx > >* const _plistdefine );
	void MakeTyp( const CStringEx _sxfullname, const CStringEx _sxtype, const CStringEx _sxformula, const CStringEx _sxenum, const CStringEx _sxlow, const CStringEx _sxhigh, const CStringEx _sxdefault, const std::list< std::pair< CStringEx, CStringEx > >* const _plistpredef, const std::list< std::pair< CStringEx, CStringEx > >* const _plistparent );
};

#endif // !defined(AFX_LOADER_H__3A379212_CAE2_48A7_84FA_3EB5D45DC7A9__INCLUDED_)
