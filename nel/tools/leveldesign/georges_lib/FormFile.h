// FormFile.h: interface for the CFormFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FORMFILE_H__51FFB53B_1A99_4E87_BCB0_7E30F1A03D4F__INCLUDED_)
#define AFX_FORMFILE_H__51FFB53B_1A99_4E87_BCB0_7E30F1A03D4F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "nel/misc/stream.h"
#include "StringEx.h"
#include "Form.h"
#include <list>

class CFormFile  
{
protected:
	std::list< CForm >	lform;

public:
	CFormFile();
	virtual ~CFormFile();
	void serial( NLMISC::IStream& s );

	void Load( const CStringEx _sxfullname );
	void Save( const CStringEx _sxfullname );
	void GetForm( CForm& _f ) const;
	void GetForm( CForm& _f, const CStringEx& _sxdate ) const; 
	void SetForm( CForm& _f );
};

#endif // !defined(AFX_FORMFILE_H__51FFB53B_1A99_4E87_BCB0_7E30F1A03D4F__INCLUDED_)

