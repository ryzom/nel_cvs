/** \file direction_attr.cpp
 * a dialog to choose a direction (normalized vector). It gives several choices, or allow 
 * to call a more complete dialog (CDirectionEdit)
 *
 * $Id: direction_attr.cpp,v 1.5.4.1 2003/06/02 11:26:00 boucher Exp $
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
#include "direction_attr.h"
#include "direction_edit.h"
#include "choose_name.h"
#include "3d/ps_direction.h"
#include "3d/particle_system.h"



/////////////////////////////////////////////////////////////////////////////
// CDirectionAttr dialog
CDirectionAttr::CDirectionAttr(const std::string &id): _DirectionDlg(NULL), _Wrapper(NULL), _DirectionWrapper(NULL)
{
	//{{AFX_DATA_INIT(CDirectionAttr)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

//=======================================================================================
void CDirectionAttr::init(uint32 x, uint32 y, CWnd *pParent)
{
	Create(IDD_DIRECTION_ATTR, pParent);
	RECT r;
	GetClientRect(&r);
	CRect wr;
	wr.left = r.left + x;
	wr.top = r.top + y;
	wr.bottom = r.bottom + y;
	wr.right = r.right + x;
	MoveWindow(wr);
	if (_DirectionWrapper && _DirectionWrapper->supportGlobalVectorValue())
	{
		GetDlgItem(IDC_GLOBAL_DIRECTION)->ShowWindow(TRUE);
	}
	EnableWindow(TRUE);
	ShowWindow(SW_SHOW);	
}


//=======================================================================================
void CDirectionAttr::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDirectionAttr)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDirectionAttr, CDialog)
	//{{AFX_MSG_MAP(CDirectionAttr)
	ON_BN_CLICKED(IDC_VECT_I, OnVectI)
	ON_BN_CLICKED(IDC_VECT_J, OnVectJ)
	ON_BN_CLICKED(IDC_VECT_K, OnVectK)
	ON_BN_CLICKED(IDC_VECT_MINUS_I, OnVectMinusI)
	ON_BN_CLICKED(IDC_VECT_MINUS_J, OnVectMinusJ)
	ON_BN_CLICKED(IDC_VECT_MINUS_K, OnVectMinusK)
	ON_BN_CLICKED(IDC_CUSTOM_DIRECTION, OnCustomDirection)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_GLOBAL_DIRECTION, OnGlobalDirection)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDirectionAttr message handlers


//=======================================================================================
void CDirectionAttr::OnVectI() 
{
	nlassert(_Wrapper);	
	_Wrapper->set(NLMISC::CVector::I);
}

//=======================================================================================
void CDirectionAttr::OnVectJ() 
{
	nlassert(_Wrapper);	
	_Wrapper->set(NLMISC::CVector::J);
}

//=======================================================================================
void CDirectionAttr::OnVectK() 
{
	_Wrapper->set(NLMISC::CVector::K);
}

//=======================================================================================
void CDirectionAttr::OnVectMinusI() 
{
	_Wrapper->set( - NLMISC::CVector::I);	
}

//=======================================================================================
void CDirectionAttr::OnVectMinusJ() 
{
	_Wrapper->set(- NLMISC::CVector::J);
}

//=======================================================================================
void CDirectionAttr::OnVectMinusK() 
{
	_Wrapper->set(- NLMISC::CVector::K);	
}	

//=======================================================================================
BOOL CDirectionAttr::EnableWindow(BOOL bEnable)
{
	BOOL enableUserDirection = TRUE;
	if (_DirectionWrapper && _DirectionWrapper->supportGlobalVectorValue() && !_DirectionWrapper->getGlobalVectorValueName().empty())
	{
		enableUserDirection = FALSE;
	}	
	GetDlgItem(IDC_VECT_I)->EnableWindow(bEnable & enableUserDirection);
	GetDlgItem(IDC_VECT_J)->EnableWindow(bEnable & enableUserDirection);
	GetDlgItem(IDC_VECT_K)->EnableWindow(bEnable & enableUserDirection);
	GetDlgItem(IDC_VECT_MINUS_I)->EnableWindow(bEnable & enableUserDirection);
	GetDlgItem(IDC_VECT_MINUS_J)->EnableWindow(bEnable & enableUserDirection);
	GetDlgItem(IDC_VECT_MINUS_K)->EnableWindow(bEnable & enableUserDirection);
	GetDlgItem(IDC_CUSTOM_DIRECTION)->EnableWindow(bEnable & enableUserDirection);
	GetDlgItem(IDC_DIRECTION_TEXT)->EnableWindow(bEnable & enableUserDirection);
	GetDlgItem(IDC_GLOBAL_DIRECTION)->EnableWindow(bEnable);
	return CEditAttribDlg::EnableWindow(bEnable);
}

//=======================================================================================
void CDirectionAttr::OnCustomDirection() 
{
#undef new
	_DirectionDlg = new CDirectionEdit(_Wrapper);
#define new NL_NEW
	_DirectionDlg->init(this, this);
	EnableWindow(FALSE);	
}

//=======================================================================================
void CDirectionAttr::childPopupClosed(CWnd *)
{
	_DirectionDlg->DestroyWindow();
	delete _DirectionDlg;
	_DirectionDlg = NULL;
	EnableWindow(TRUE);

}

//=======================================================================================
void CDirectionAttr::OnDestroy() 
{
	CDialog::OnDestroy();	
	if (_DirectionDlg)
	{
		_DirectionDlg->DestroyWindow();
	}
	delete _DirectionDlg;
}

//=======================================================================================
void CDirectionAttr::OnGlobalDirection() 
{
	nlassert(_DirectionWrapper)

	CChooseName cn(_DirectionWrapper->getGlobalVectorValueName().c_str());
	if (cn.DoModal() == IDOK)
	{
		_DirectionWrapper->enableGlobalVectorValue(cn.getName());
		if (!cn.getName().empty())
		{
			NL3D::CParticleSystem::setGlobalVectorValue(cn.getName(), NLMISC::CVector::I); // take a non NULL value for the direction
		}
	}
	EnableWindow(TRUE);
}
