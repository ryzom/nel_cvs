// Loader.h: interface for the CLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_LOADER_H__3A379212_CAE2_48A7_84FA_3EB5D45DC7A9__INCLUDED_)
#define AFX_LOADER_H__3A379212_CAE2_48A7_84FA_3EB5D45DC7A9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "nel/misc/path.h"
#include "FormLoader.h"
#include "MoldLoader.h"

class CLoader  
{
protected:
	CFormLoader fl;
	CMoldLoader ml;
	CStringEx sxworkdirectory;
	CStringEx sxrootdirectory;
	CStringEx WhereIs( const CStringEx _sxdirectory, const CStringEx _sxfilename );

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

	CStringEx WhereIsDfnTyp( const CStringEx _sxfilename );
	CStringEx WhereIsForm( const CStringEx _sxfilename );

	CStringEx GetWorkDirectory() const;
	CStringEx GetRootDirectory() const;

// interface:
	void SetWorkDirectory( const CStringEx _sxworkdirectory );
	void SetRootDirectory( const CStringEx _sxrootdirectory );
	void SetTypPredef( const CStringEx _sxfilename, const std::vector< CStringEx >& _pvsx );

// temporaire:
	void MakeDfn( const CStringEx _sxfullname, const std::vector< std::pair< CStringEx, CStringEx > >* const _pvdefine = 0 );
	void MakeTyp( const CStringEx _sxfullname, const CStringEx _sxtype, const CStringEx _sxformula, const CStringEx _sxenum, const CStringEx _sxlow, const CStringEx _sxhigh, const CStringEx _sxdefault, const std::vector< std::pair< CStringEx, CStringEx > >* const _pvpredef = 0, const std::vector< std::pair< CStringEx, CStringEx > >* const _pvparent = 0 );
};

#endif // !defined(AFX_LOADER_H__3A379212_CAE2_48A7_84FA_3EB5D45DC7A9__INCLUDED_)
