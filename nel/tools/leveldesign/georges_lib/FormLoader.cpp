// FormLoader.cpp: implementation of the CFormLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FormLoader.h"

/*#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif*/

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormLoader::CFormLoader()
{
}

CFormLoader::~CFormLoader()
{
}

void CFormLoader::LoadForm( CForm& _f, const CStringEx& _sxfilename )
{
	CFormFile* pff = new CFormFile;
	pff->Load( _sxfilename );
	pff->GetForm( _f );
	delete pff;
}

void CFormLoader::LoadForm( CForm& _f, const CStringEx& _sxfilename, const CStringEx& _sxdate ) 
{
	CFormFile* pff = new CFormFile;
	pff->Load( _sxfilename );
	pff->GetForm( _f, _sxdate );
	delete pff;
}

void CFormLoader::SaveForm( CForm& _f, const CStringEx& _sxfilename )
{
	CFormFile* pff = new CFormFile;
	pff->SetForm( _f );
	pff->Save( _sxfilename );
	delete pff;
}


