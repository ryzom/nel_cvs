/** \file located_bindable_dialog.cpp
 * <File description>
 *
 * $Id: located_bindable_dialog.cpp,v 1.2 2001/06/12 17:12:36 vizerie Exp $
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
 * MA 02111-1307, USA.
 */

#include "std_afx.h"
#include "object_viewer.h"
#include "located_bindable_dialog.h"


#include "nel/3d/ps_located.h"
#include "nel/3d/ps_particle.h"
#include "nel/3d/ps_force.h"
#include "nel/3d/ps_emitter.h"
#include "nel/3d/ps_zone.h"

#include "attrib_dlg.h"


using NL3D::CPSLocatedBindable ; 

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif





/////////////////////////////////////////////////////////////////////////////
// CLocatedBindableDialog dialog


CLocatedBindableDialog::CLocatedBindableDialog(NL3D::CPSLocatedBindable *bindable)
	: _Bindable(bindable)
{
	//{{AFX_DATA_INIT(CLocatedBindableDialog)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


CLocatedBindableDialog::~CLocatedBindableDialog()
{
	for (std::vector<CDialog *>::iterator it = _SubDialogs.begin() ; it != _SubDialogs.end() ; ++it)
	{
		delete *it ;
	}
}

void CLocatedBindableDialog::init(CWnd* pParent)
{
	Create(IDD_LOCATED_BINDABLE, pParent) ;
	ShowWindow(SW_SHOW) ;

	uint yPos = 35 ;
	const uint xPos = 5 ;
	RECT rect ;


	if (dynamic_cast<NL3D::CPSParticle *>(_Bindable))
	{
		NL3D::CPSParticle *p = (NL3D::CPSParticle *) _Bindable ;

		// check support for color
		if (dynamic_cast<NL3D::CPSColoredParticle *>(_Bindable))
		{
			
			CAttribDlgRGBA *ad = new CAttribDlgRGBA("PARTICLE_COLOR") ;
			_SubDialogs.push_back(ad) ;

			_ColorWrapper.S = dynamic_cast<NL3D::CPSColoredParticle *>(_Bindable) ;		
			ad->setWrapper(&_ColorWrapper) ;			
			ad->setSchemeWrapper(&_ColorWrapper) ;

			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_COLOR)) ;
			ad->init(bmh, xPos, yPos, this) ;
			ad->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;
		}

		// check support for size
		if (dynamic_cast<NL3D::CPSSizedParticle *>(_Bindable))
		{
			
			CAttribDlgFloat *ad = new CAttribDlgFloat("PARTICLE_SIZE", 0.f, 10.f) ;
			_SubDialogs.push_back(ad) ;

			_SizeWrapper.S = dynamic_cast<NL3D::CPSSizedParticle *>(_Bindable) ;
			ad->setWrapper(&_SizeWrapper) ;
			ad->setSchemeWrapper(&_SizeWrapper) ;
					
			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_SIZE)) ;
			ad->init(bmh, xPos, yPos, this) ;
			ad->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;
		}


		// check support for angle 2D
		if (dynamic_cast<NL3D::CPSRotated2DParticle *>(_Bindable))
		{
			
			CAttribDlgFloat *ad = new CAttribDlgFloat("PARTICLE_ANGLE2D", 0.f, 256.f) ;
			_SubDialogs.push_back(ad) ;

			_Angle2DWrapper.S = dynamic_cast<NL3D::CPSRotated2DParticle *>(_Bindable) ;
			ad->setWrapper(&_Angle2DWrapper) ;						
			ad->setSchemeWrapper(&_Angle2DWrapper) ;	

			HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_PARTICLE_ANGLE)) ;
			ad->init(bmh, xPos, yPos, this) ;
			ad->GetClientRect(&rect) ;
			yPos += rect.bottom + 3 ;
		}
	}


	
}


void CLocatedBindableDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLocatedBindableDialog)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLocatedBindableDialog, CDialog)
	//{{AFX_MSG_MAP(CLocatedBindableDialog)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CLocatedBindableDialog message handlers
