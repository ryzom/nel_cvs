/** \file texture_chooser.cpp
 * A dailog that helps to choose particles texture
 * $Id: texture_chooser.cpp,v 1.3 2001/06/25 12:54:57 vizerie Exp $
 */

/* Copyright, 2000 Nevrax Ltd.
 *
 * This file is part of NEVRAX NEL.
 * NEVRAX NEL is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.

 * NEVRAX NEL is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with NEVRAX NEL; see the file COPYING. If not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330, Boston,
 * MA 02
*/

#include "std_afx.h"
#include "object_viewer.h"
#include "texture_chooser.h"

#include "3d/texture_file.h"
#include "nel/misc/path.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// size of the bitmap that is displayed
const uint tSize = 25 ;

/////////////////////////////////////////////////////////////////////////////
// CTextureChooser dialog


CTextureChooser::CTextureChooser() : _CurrBitmap(0), _Wrapper(NULL), _Texture(NULL)
{
	//{{AFX_DATA_INIT(CTextureChooser)
	//}}AFX_DATA_INIT
}

CTextureChooser::~CTextureChooser()
{
	if (_CurrBitmap)
		::DeleteObject(_CurrBitmap) ;
}


void CTextureChooser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTextureChooser)
	//}}AFX_DATA_MAP
}


void CTextureChooser::init(uint x, uint y, CWnd *pParent)
{
	Create(IDD_TEXTURE_CHOOSER, pParent) ;
	RECT r ;
	GetClientRect(&r) ;
	MoveWindow(x, y, r.right, r.bottom) ;	

	ShowWindow(SW_SHOW) ;
}


void CTextureChooser::textureToBitmap()
{
	if (!_Texture) return ;
	
	if (_CurrBitmap)
	{
		::DeleteObject(_CurrBitmap) ;	
	}
	


	_Texture->generate() ;


	// make copy of the texture
	NLMISC::CBitmap cb(* ((NL3D::ITexture *) _Texture)) ;

	cb.resample(tSize, tSize) ;
	cb.convertToType(NLMISC::CBitmap::RGBA) ;

	uint32 *dat  = (uint32 *) &(cb.getPixels()[0]) ;

	_CurrBitmap = ::CreateBitmap(tSize, tSize, 1, 32, dat) ;
	

	
	_Texture->release() ;


	Invalidate() ;

	UpdateData(TRUE) ;
}



BEGIN_MESSAGE_MAP(CTextureChooser, CDialog)
	//{{AFX_MSG_MAP(CTextureChooser)
	ON_BN_CLICKED(IDC_BROWSE_TEXTURE, OnBrowseTexture)
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTextureChooser message handlers

BOOL CTextureChooser::OnInitDialog() 
{
	CDialog::OnInitDialog();	
	nlassert(_Wrapper) ;	
	_Texture = _Wrapper->get() ;

	// generate the bitmap
	
	textureToBitmap() ;
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CTextureChooser::OnBrowseTexture() 
{
	CFileDialog fd(TRUE, ".tga", "*.tga", 0, NULL, this) ;
	if (fd.DoModal() == IDOK)
	{
		// Add to the path
		char drive[256];
		char dir[256];
		char path[256];

		// Add search path for the texture
		_splitpath (fd.GetPathName(), drive, dir, NULL, NULL);
		_makepath (path, drive, dir, NULL, NULL);
		NLMISC::CPath::addSearchPath (path);

		try
		{
			NL3D::CTextureFile *tf = new NL3D::CTextureFile(std::string(fd.GetFileName())) ;
			_Wrapper->set(tf) ;
			_Texture = tf ;
			textureToBitmap() ;
		}
		catch (NLMISC::Exception &e)
		{
			MessageBox(e.what(), "error loading texture") ;
		}		
	
	}
}

void CTextureChooser::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	if (_CurrBitmap)
	{
		HDC bitmapDc = ::CreateCompatibleDC(dc) ;
		HGDIOBJ old = ::SelectObject(bitmapDc, _CurrBitmap) ;
		
		::BitBlt(dc, 10, 10, tSize, tSize, bitmapDc, 0, 0, SRCCOPY) ;

		::SelectObject(bitmapDc, old) ;
		::DeleteDC(bitmapDc) ;
	}
}
