/** \file scene_dlg.cpp
 * <File description>
 *
 * $Id: scene_dlg.cpp,v 1.7 2001/04/30 16:58:31 corvazier Exp $
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
#include "scene_dlg.h"
#include <nel/misc/file.h>
#include <nel/3d/nelu.h>
#include <nel/3d/mesh.h>
#include <nel/3d/transform_shape.h>
#include <nel/3d/mesh_instance.h>
#include <nel/3d/skeleton_model.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NLMISC;
using namespace NL3D;

/////////////////////////////////////////////////////////////////////////////
// CSceneDlg dialog


CSceneDlg::CSceneDlg(CObjectViewer *objView, CWnd* pParent /*=NULL*/)
	: CDialog(CSceneDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSceneDlg)
	ViewAnimation = FALSE;
	ViewAnimationSet = FALSE;
	ViewSlots = FALSE;
	Euler = FALSE;
	ObjectMode = TRUE;
	MoveSpeed = 10.0f;
	//}}AFX_DATA_INIT
	ObjView=objView;

	// Get value from the register
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_OBJ_VIEW_SCENE_DLG, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
	{
		DWORD len=sizeof (BOOL);
		DWORD type;
		RegQueryValueEx (hKey, "ViewAnimation", 0, &type, (LPBYTE)&ViewAnimation, &len);
		len=sizeof (BOOL);
		RegQueryValueEx (hKey, "ViewAnimationSet", 0, &type, (LPBYTE)&ViewAnimationSet, &len);
		len=sizeof (BOOL);
		RegQueryValueEx (hKey, "ViewSlots", 0, &type, (LPBYTE)&ViewSlots, &len);
		len=sizeof (float);
		RegQueryValueEx (hKey, "MoveSpeed", 0, &type, (LPBYTE)&MoveSpeed, &len);
		len=sizeof (BOOL);
		RegQueryValueEx (hKey, "ObjectMode", 0, &type, (LPBYTE)&ObjectMode, &len);
	}
}

CSceneDlg::~CSceneDlg()
{
	HKEY hKey;
	if (RegCreateKey(HKEY_CURRENT_USER, REGKEY_OBJ_VIEW_SCENE_DLG, &hKey)==ERROR_SUCCESS)
	{
		RegSetValueEx(hKey, "ViewAnimation", 0, REG_BINARY, (LPBYTE)&ViewAnimation, sizeof(bool));
		RegSetValueEx(hKey, "ViewAnimationSet", 0, REG_BINARY, (LPBYTE)&ViewAnimationSet, sizeof(bool));
		RegSetValueEx(hKey, "ViewSlots", 0, REG_BINARY, (LPBYTE)&ViewSlots, sizeof(bool));
		RegSetValueEx(hKey, "MoveSpeed", 0, REG_BINARY, (LPBYTE)&MoveSpeed, sizeof(float));
		RegSetValueEx(hKey, "ObjectMode", 0, REG_BINARY, (LPBYTE)&ObjectMode, sizeof(BOOL));
	}
}

void CSceneDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSceneDlg)
	DDX_Control(pDX, IDC_MOVE_SPEED, MoveSpeedCtrl);
	DDX_Check(pDX, IDC_VIEW_ANIMATION, ViewAnimation);
	DDX_Check(pDX, IDC_VIEW_ANIMATIONSET, ViewAnimationSet);
	DDX_Check(pDX, IDC_VIEW_SLOTS, ViewSlots);
	DDX_Check(pDX, IDC_EULER, Euler);
	DDX_Check(pDX, IDC_OBJECT_MODE, ObjectMode);
	DDX_Text(pDX, IDC_MOVE_SPEED, MoveSpeed);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSceneDlg, CDialog)
	//{{AFX_MSG_MAP(CSceneDlg)
	ON_BN_CLICKED(IDC_CLEAR_SCENE, OnClearScene)
	ON_BN_CLICKED(IDC_LOAD_MESH, OnLoadMesh)
	ON_BN_CLICKED(IDC_LOAD_PLAYLIST, OnLoadPlaylist)
	ON_BN_CLICKED(IDC_LOAD_SCENE, OnLoadScene)
	ON_BN_CLICKED(IDC_SAVE_PLAYLIST, OnSavePlaylist)
	ON_BN_CLICKED(IDC_VIEW_ANIMATION, OnViewAnimation)
	ON_BN_CLICKED(IDC_VIEW_ANIMATIONSET, OnViewAnimationset)
	ON_BN_CLICKED(IDC_VIEW_SLOTS, OnViewSlots)
	ON_BN_CLICKED(IDC_RESET_CAMERA, OnResetCamera)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSceneDlg message handlers

void CSceneDlg::OnClearScene() 
{
	// *** Clear the scene.

	// Remove all the instance
	uint i;
	for (i=0; i<ObjView->_ListTransformShape.size(); i++)
		CNELU::Scene.deleteInstance (ObjView->_ListTransformShape[i]);

	// Clear the pointer array
	ObjView->_ListTransformShape.clear ();

	// Erase the channel mixer
	ObjView->_ChannelMixer.resetChannels ();

	// Erase the entry of the viewer
	ObjView->_ListMeshes.clear ();
}

void CSceneDlg::OnLoadMesh() 
{
	// Update UI
	UpdateData ();

	// Create a dialog
	static char BASED_CODE szFilter[] = "NeL Shape Files (*.shape)|*.shape|All Files (*.*)|*.*||";
	CFileDialog fileDlg( TRUE, ".shape", "*.shape", OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
	if (fileDlg.DoModal()==IDOK)
	{

		// Create a dialog
		static char BASED_CODE szFilter2[] = "NeL Skeleton Files (*.skel)|*.skel|All Files (*.*)|*.*||";
		CFileDialog fileDlg2 ( TRUE, ".skel", "*.skel", OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter2);
		if (fileDlg2.DoModal()==IDOK)
		{
			// Load the shape with a skeleton
			if (ObjView->loadMesh (fileDlg.GetPathName(), fileDlg2.GetPathName()))
			{
				// Reset the camera
				OnResetCamera();

				// Touch the channel mixer
				ObjView->reinitChannels ();
			}
		}
		else
		{
			// Load the shape without skeleton
			if (ObjView->loadMesh (fileDlg.GetPathName(), ""))
			{
				// Reset the camera
				OnResetCamera();

				// Touch the channel mixer
				ObjView->reinitChannels ();
			}
		}
	}
}

void CSceneDlg::OnLoadPlaylist() 
{
	// Update UI
	UpdateData ();

	// Create a dialog
	static char BASED_CODE szFilter[] = "NeL Object viewer config (*.ovcgf)|*.ovcgf|All Files (*.*)|*.*||";
	CFileDialog fileDlg( TRUE, ".ovcgf", "*.ovcgf", OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
	if (fileDlg.DoModal()==IDOK)
	{
		// Open the file
		CIFile file;
		if (file.open ((const char*)fileDlg.GetPathName()))
		{
			try
			{
				ObjView->serial (file);
			}
			catch (Exception& e)
			{
				MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
			}
		}
		else
		{
			// Create a message
			char msg[512];
			_snprintf (msg, 512, "Can't open the file %s for reading.", (const char*)fileDlg.GetPathName());
			MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
		}
	}
}

void CSceneDlg::OnLoadScene() 
{
	// TODO: Add your control notification handler code here
	
}

void CSceneDlg::OnSavePlaylist() 
{
	// Update UI
	UpdateData ();

	// Create a dialog
	static char BASED_CODE szFilter[] = "NeL Object viewer config (*.ovcgf)|*.ovcgf|All Files (*.*)|*.*||";
	CFileDialog fileDlg( FALSE, ".ovcgf", "*.ovcgf", OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
	if (fileDlg.DoModal()==IDOK)
	{
		// Open the file
		COFile file;
		if (file.open ((const char*)fileDlg.GetPathName()))
		{
			try
			{
				ObjView->serial (file);
			}
			catch (Exception& e)
			{
				MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
			}
		}
		else
		{
			// Create a message
			char msg[512];
			_snprintf (msg, 512, "Can't open the file %s for writing", (const char*)fileDlg.GetPathName());
			MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
		}
	}
}

void CSceneDlg::OnViewAnimation() 
{
	UpdateData ();
	if (ObjView->_AnimationDlg)
		ObjView->_AnimationDlg->ShowWindow (ViewAnimation?SW_SHOW:SW_HIDE);
}

void CSceneDlg::OnViewAnimationset() 
{
	UpdateData ();
	if (ObjView->_AnimationSetDlg)
		ObjView->_AnimationSetDlg->ShowWindow (ViewAnimationSet?SW_SHOW:SW_HIDE);
}

void CSceneDlg::OnViewSlots() 
{
	UpdateData ();
	if (ObjView->_SlotDlg)
		ObjView->_SlotDlg->ShowWindow (ViewSlots?SW_SHOW:SW_HIDE);
}

BOOL CSceneDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	// Init dlg box
	OnViewAnimation();
	OnViewAnimationset();
	OnViewSlots();
	OnResetCamera();

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CSceneDlg::OnResetCamera() 
{
	// One object found at least
	bool found=false;
	
	// Pointer on the CMesh;
	CVector hotSpot=CVector (0,0,0);

	// Reset the radius
	float radius=10.f;

	// Look for a first mesh
	uint m;
	for (m=0; m<ObjView->_ListTransformShape.size(); m++)
	{
		CMeshInstance *pTransform=dynamic_cast<CMeshInstance*>(ObjView->_ListTransformShape[m]);
		if (pTransform)
		{
			IShape *pShape=pTransform->Shape;
			CMesh *pMesh=(CMesh*)pShape;
			CSkeletonModel *pSkelModel=pTransform->getSkeletonModel ();

			if (!pSkelModel)
			{
				// Reset the hotspot
				hotSpot=pTransform->getMatrix()*pMesh->getBoundingBox().getCenter();

				// Reset the radius
				radius=pMesh->getBoundingBox().getRadius();
				radius=pTransform->getMatrix().mulVector (CVector (radius, 0, 0)).norm();
				found=true;
				m++;
				break;
			}
			else
			{
				// Get first bone
				if (pSkelModel->Bones.size())
				{
					// Ok, it is the root.
					hotSpot=pSkelModel->Bones[0].getMatrix()*pMesh->getBoundingBox().getCenter();

					// Reset the radius
					radius=pMesh->getBoundingBox().getRadius();
					radius=pSkelModel->Bones[0].getMatrix().mulVector (CVector (radius, 0, 0)).norm();
					found=true;
					m++;
					break;
				}
			}
		}
	}

	// For each model in the list
	for (; m<ObjView->_ListTransformShape.size(); m++)
	{
		// Pointer on the CMesh;
		CMeshInstance *pTransform=dynamic_cast<CMeshInstance*>(ObjView->_ListTransformShape[m]);
		if (pTransform)
		{
			IShape *pShape=pTransform->Shape;
			CMesh *pMesh=(CMesh*)pShape;
			CSkeletonModel *pSkelModel=pTransform->getSkeletonModel ();

			// New radius and hotSpot
			CVector hotSpot2;
			float radius2;
			bool setuped=false;

			if (!pSkelModel)
			{
				// Get the hotspot
				hotSpot2=pTransform->getMatrix()*pMesh->getBoundingBox().getCenter();

				// Get the radius
				radius2=pMesh->getBoundingBox().getRadius();
				radius2=pTransform->getMatrix().mulVector (CVector (radius2, 0, 0)).norm();

				// Ok found it
				setuped=true;
			}
			else
			{
				// Get first bone
				if (pSkelModel->Bones.size())
				{
					// Get the hotspot
					hotSpot2=pSkelModel->Bones[0].getMatrix()*pMesh->getBoundingBox().getCenter();

					// Get the radius
					radius2=pMesh->getBoundingBox().getRadius();
					radius2=pSkelModel->Bones[0].getMatrix().mulVector (CVector (radius2, 0, 0)).norm();

					// Ok found it
					setuped=true;
				}
			}

			if (setuped)
			{
				// *** Merge with previous

				// Get vector center to center
				CVector vect=hotSpot-hotSpot2;
				vect.normalize();
				
				// Get the right position
				CVector right=hotSpot+vect*radius;
				if ((right-hotSpot2).norm()<radius2)
					right=hotSpot2+vect*radius2;
				
				// Get the left position
				CVector left=hotSpot2-vect*radius2;
				if ((left-hotSpot).norm()<radius)
					left=hotSpot-vect*radius;

				// Get new center
				hotSpot=(left+right)/2.f;

				// Get new size
				radius=(left-right).norm()/2.f;
			}
		}
	}

	// Setup camera
	CNELU::Camera->lookAt (hotSpot+CVector(0.57735f,0.57735f,0.57735f)*radius, hotSpot);

	// Setup mouse listener
	ObjView->_MouseListener.setMatrix (CNELU::Camera->getMatrix());
	ObjView->_MouseListener.setFrustrum (CNELU::Camera->getFrustum());
	ObjView->_MouseListener.setViewport (CViewport());
	ObjView->_MouseListener.setHotSpot (hotSpot);
	ObjView->_MouseListener.setMouseMode (CEvent3dMouseListener::edit3d);
}

void CSceneDlg::OnDestroy() 
{
	// TODO: Add your message handler code here
	setRegisterWindowState (this, REGKEY_OBJ_VIEW_SCENE_DLG);

	CDialog::OnDestroy();
}
