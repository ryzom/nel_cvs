// FormLoader.h: interface for the CFormLoader class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_FORMLOADER_H__8E718C2F_7CBC_4CB5_AD3E_741A59DCABF6__INCLUDED_)
#define AFX_FORMLOADER_H__8E718C2F_7CBC_4CB5_AD3E_741A59DCABF6__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "StringEx.h"
#include "FormFile.h"
#include < map >

class CFormLoader  
{
protected:
//	std::map< CStringEx, CFormFile* >;

public:
	CFormLoader();
	virtual ~CFormLoader();

	void LoadForm( CForm& _f, const CStringEx& _sxfilename );
	void LoadForm( CForm& _f, const CStringEx& _sxfilename, const CStringEx& _sxdate ); 
	void SaveForm( CForm& _f, const CStringEx& _sxfilename );
};

#endif // !defined(AFX_FORMLOADER_H__8E718C2F_7CBC_4CB5_AD3E_741A59DCABF6__INCLUDED_)
