// FormLoader.cpp: implementation of the CFormLoader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FormLoader.h"

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
	if( _sxfilename.empty() )
		return;
	CFormFile* pff = new CFormFile;
	pff->Load( _sxfilename );
	pff->GetForm( _f );
	delete pff;
}

void CFormLoader::LoadForm( CForm& _f, const CStringEx& _sxfilename, const CStringEx& _sxdate ) 
{
	if( _sxfilename.empty() )
		return;
	CFormFile* pff = new CFormFile;
	pff->Load( _sxfilename );
	pff->GetForm( _f, _sxdate );
	delete pff;
}

void CFormLoader::SaveForm( CForm& _f, const CStringEx& _sxfilename )
{
	if( _sxfilename.empty() )
		return;
	CFormFile* pff = new CFormFile;
	pff->SetForm( _f );
	pff->Save( _sxfilename );
	delete pff;
}


