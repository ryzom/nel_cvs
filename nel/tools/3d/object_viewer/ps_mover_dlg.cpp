/** \file ps_mover_dlg.cpp
 * this dialog display coordinate of an instance of a located in a particle system 
 * $Id: ps_mover_dlg.cpp,v 1.4 2001/06/25 13:00:37 vizerie Exp $
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
#include "ps_mover_dlg.h"
#include "particle_tree_ctrl.h"
#include "3d/ps_located.h"

#include "editable_range.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CPSMoverDlg dialog


CPSMoverDlg::CPSMoverDlg(CParticleTreeCtrl *parent, NL3D::CEvent3dMouseListener *ml,  HTREEITEM editedItem)   // standard constructor
	: CDialog(CPSMoverDlg::IDD, parent)
	  , _MouseListener(ml)
	  ,	_Scale(NULL), _XScale(NULL), _YScale(NULL), _ZScale(NULL)
	  , _ScaleText(NULL), _XScaleText(NULL), _YScaleText(NULL), _ZScaleText(NULL)
{

	_TreeCtrl = parent ;
	_EditedNode = (CParticleTreeCtrl::CNodeType *) parent->GetItemData(editedItem) ;

	nlassert(_EditedNode->Type == CParticleTreeCtrl::CNodeType::locatedInstance) ;

	const NLMISC::CVector &pos = _EditedNode->Loc->getPos()[_EditedNode->LocatedInstanceIndex] ;	
	
	m_X.Format("%.3g", pos.x) ;
	m_Y.Format("%.3g", pos.y) ;
	m_Z.Format("%.3g", pos.z) ;

	//{{AFX_DATA_INIT(CPSMoverDlg)
	//}}AFX_DATA_INIT


}

CPSMoverDlg::~CPSMoverDlg()
{
	cleanScaleCtrl() ;
}

void CPSMoverDlg::updateListener(void)
{
	NLMISC::CVector &pos = _EditedNode->Loc->getPos()[_EditedNode->LocatedInstanceIndex] ;
	NLMISC::CMatrix m ;
	m = _MouseListener->getModelMatrix() ;
	m.setPos(pos) ;
	_MouseListener->setModelMatrix(m) ;

}

void CPSMoverDlg::updatePosition(void)
{
	UpdateData() ;
	const NLMISC::CVector &pos = _EditedNode->Loc->getPos()[_EditedNode->LocatedInstanceIndex] ;	
	
	m_X.Format("%.3g", pos.x) ;
	m_Y.Format("%.3g", pos.y) ;
	m_Z.Format("%.3g", pos.z) ;

	UpdateData(FALSE) ;
}

void CPSMoverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPSMoverDlg)
	DDX_Control(pDX, IDC_SUB_COMPONENT, m_SubComponentCtrl);
	DDX_Text(pDX, IDC_XPOS, m_X);
	DDX_Text(pDX, IDC_YPOS, m_Y);
	DDX_Text(pDX, IDC_ZPOS, m_Z);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPSMoverDlg, CDialog)
	//{{AFX_MSG_MAP(CPSMoverDlg)
	ON_BN_CLICKED(IDC_UPDATE_XPOS, OnUpdateXpos)
	ON_BN_CLICKED(IDC_UPDATE_YPOS, OnUpdateYpos)
	ON_BN_CLICKED(IDC_UPDATE_ZPOS, OnUpdateZpos)
	ON_LBN_SELCHANGE(IDC_SUB_COMPONENT, OnSelchangeSubComponent)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPSMoverDlg message handlers

void CPSMoverDlg::OnUpdateXpos() 
{
	UpdateData() ;
	NLMISC::CVector &pos = _EditedNode->Loc->getPos()[_EditedNode->LocatedInstanceIndex] ;
	float x ;
	if (::sscanf(m_X, "%f", &x) == 1)
	{
		pos.x = x ;
		updateListener() ;
	}
	else
	{
		MessageBox("invalid entry", "error") ;
	}
	UpdateData(FALSE) ;
}

void CPSMoverDlg::OnUpdateYpos() 
{
	UpdateData() ;
	NLMISC::CVector &pos = _EditedNode->Loc->getPos()[_EditedNode->LocatedInstanceIndex] ;
	float y ;
	if (::sscanf(m_Y, "%f", &y) == 1)
	{
		pos.y = y ;
		updateListener() ;
	}
	else
	{
		MessageBox("invalid entry", "error") ;
	}
	UpdateData(FALSE) ;	
}

void CPSMoverDlg::OnUpdateZpos() 
{
	UpdateData() ;
	NLMISC::CVector &pos = _EditedNode->Loc->getPos()[_EditedNode->LocatedInstanceIndex] ;
	float z ;
	if (::sscanf(m_Z, "%f", &z) == 1)
	{
		pos.z = z ;
		updateListener() ;		
	}
	else
	{
		MessageBox("invalid entry", "error") ;
	}
	UpdateData(FALSE) ;	
}

BOOL CPSMoverDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	nlassert(_EditedNode) ;
	uint numBound = _EditedNode->Loc->getNbBoundObjects() ;
	
	uint nbCandidates = 0 ;

	for (uint k = 0 ; k < numBound ; ++k)
	{
		if (dynamic_cast<NL3D::IPSMover *>(_EditedNode->Loc->getBoundObject(k)))
		{
			uint insertedLine = m_SubComponentCtrl.AddString(_EditedNode->Loc->getBoundObject(k)->getName().c_str()) ;
			m_SubComponentCtrl.SetItemData(insertedLine, (DWORD) _EditedNode->Loc->getBoundObject(k)) ;
			++nbCandidates ;			
		}
	}

	if (nbCandidates > 0)
	{
		m_SubComponentCtrl.SetCurSel(0) ;	
		_EditedNode->Loc->getOwner()->setCurrentEditedElement(_EditedNode->Loc, _EditedNode->LocatedInstanceIndex, (NL3D::CPSLocatedBindable *) m_SubComponentCtrl.GetItemData(0)) ;
	}
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPSMoverDlg::cleanScaleCtrl(void)
{
	#define REMOVE_SCALE_WINDOW(w) if (w) { w->DestroyWindow() ; delete w ; w = NULL ; }
									
	REMOVE_SCALE_WINDOW(_Scale) ;
	REMOVE_SCALE_WINDOW(_XScale) ;
	REMOVE_SCALE_WINDOW(_YScale) ;
	REMOVE_SCALE_WINDOW(_ZScale) ;

	REMOVE_SCALE_WINDOW(_ScaleText) ;
	REMOVE_SCALE_WINDOW(_XScaleText) ;
	REMOVE_SCALE_WINDOW(_YScaleText) ;
	REMOVE_SCALE_WINDOW(_ZScaleText) ;
}

void CPSMoverDlg::createScaleControls(void)
{
	cleanScaleCtrl() ;

	if (m_SubComponentCtrl.GetCurSel() == -1) return ;
	
	NL3D::CPSLocatedBindable *lb = (NL3D::CPSLocatedBindable *) m_SubComponentCtrl.GetItemData(m_SubComponentCtrl.GetCurSel()) ;
	NL3D::IPSMover *m = dynamic_cast<NL3D::IPSMover *>(lb) ;

	const sint xPos = 10 ;
	sint yPos = 330 ;
	

	if (m->supportUniformScaling() && ! m->supportNonUniformScaling() )
	{
		_Scale = new CEditableRangeFloat("UNIFORM SCALE", 0.f, 4.f)  ;
		_UniformScaleWrapper.M = m ;
		_UniformScaleWrapper.Index = _EditedNode->LocatedInstanceIndex ;

		_Scale->setWrapper(&_UniformScaleWrapper) ;
		_Scale->init(100 + xPos, yPos, this) ;

		_ScaleText = new CStatic ;
		_ScaleText ->Create("Scale : ", SS_LEFT, CRect(xPos, yPos + 10, xPos + 60, yPos + 32), this) ;
		_ScaleText ->ShowWindow(SW_SHOW) ;
	}
	else if (m->supportNonUniformScaling())
	{
		RECT r ;

		// dialog for edition of x scale
		_XScale = new CEditableRangeFloat("X SCALE", 0.f, 4.f)  ;
		_XScaleWrapper.M = m ;
		_XScaleWrapper.Index = _EditedNode->LocatedInstanceIndex ;
		_XScale->setWrapper(&_XScaleWrapper) ;
		_XScale->init(xPos + 70, yPos, this) ;
		_XScale->GetClientRect(&r) ;

		_XScaleText = new CStatic ;
		_XScaleText ->Create("x Scale : ", SS_LEFT, CRect(xPos, yPos + 10, xPos + 60, yPos + 32), this) ;
		_XScaleText ->ShowWindow(SW_SHOW) ;

		yPos += r.bottom ;

		// dialog for edition of y scale
		_YScale = new CEditableRangeFloat("Y SCALE", 0.f, 4.f)  ;
		_YScaleWrapper.M = m ;
		_YScaleWrapper.Index = _EditedNode->LocatedInstanceIndex ;
		_YScale->setWrapper(&_YScaleWrapper) ;
		_YScale->init(xPos + 70, yPos, this) ;
		_YScale->GetClientRect(&r) ;
		

		_YScaleText = new CStatic ;
		_YScaleText ->Create("y Scale : ", SS_LEFT, CRect(xPos, yPos + 10, xPos + 60, yPos + 32), this) ;
		_YScaleText ->ShowWindow(SW_SHOW) ;

		yPos += r.bottom ;

		// dialog for edition of x scale
		_ZScale = new CEditableRangeFloat("Z SCALE", 0.f, 4.f)  ;
		_ZScaleWrapper.M = m ;
		_ZScaleWrapper.Index = _EditedNode->LocatedInstanceIndex ;
		_ZScale->setWrapper(&_ZScaleWrapper) ;
		_ZScale->init(xPos + 70, yPos, this) ;
		_ZScale->GetClientRect(&r) ;
		
		_ZScaleText = new CStatic ;
		_ZScaleText ->Create("z Scale : ", SS_LEFT, CRect(xPos, yPos + 10, xPos + 60, yPos + 32), this) ;
		_ZScaleText ->ShowWindow(SW_SHOW) ;


	}




	
}

void CPSMoverDlg::OnSelchangeSubComponent() 
{
	UpdateData() ;
	nlassert(_EditedNode) ;

	NL3D::CPSLocatedBindable *lb = (NL3D::CPSLocatedBindable *) m_SubComponentCtrl.GetItemData(m_SubComponentCtrl.GetCurSel()) ;
	_EditedNode->LocMover = dynamic_cast<NL3D::IPSMover *>(lb) ;
	_EditedNode->Loc->getOwner()->setCurrentEditedElement(_EditedNode->Loc, _EditedNode->LocatedInstanceIndex, lb) ;
	
	createScaleControls() ;

	UpdateData(FALSE) ;
}

void CPSMoverDlg::init(CWnd *parent)
{

	Create(CPSMoverDlg::IDD, parent) ;
	ShowWindow(SW_SHOW) ;
	createScaleControls() ;
}
