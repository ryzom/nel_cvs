// scene_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "scene_dlg.h"
#include <nel/misc/file.h>
#include <nel/3d/nelu.h>
#include <nel/3d/mesh.h>
#include <nel/3d/transform_shape.h>

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
	//}}AFX_DATA_INIT
	ObjView=objView;

	// Get value from the register
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, REGKEY_OBJ_VIEW_SCENE_DLG, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
	{
		DWORD len=sizeof (BOOL);
		DWORD type;
		RegQueryValueEx (hKey, "ViewAnimation", 0, &type, (LPBYTE)&ViewAnimation, &len);
		RegQueryValueEx (hKey, "ViewAnimationSet", 0, &type, (LPBYTE)&ViewAnimationSet, &len);
		RegQueryValueEx (hKey, "ViewSlots", 0, &type, (LPBYTE)&ViewSlots, &len);
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
	}
}

void CSceneDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSceneDlg)
	DDX_Check(pDX, IDC_VIEW_ANIMATION, ViewAnimation);
	DDX_Check(pDX, IDC_VIEW_ANIMATIONSET, ViewAnimationSet);
	DDX_Check(pDX, IDC_VIEW_SLOTS, ViewSlots);
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
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSceneDlg message handlers

void CSceneDlg::OnClearScene() 
{
	// TODO: Add your control notification handler code here
	
}

void CSceneDlg::OnLoadMesh() 
{
	// Create a dialog
	static char BASED_CODE szFilter[] = "NeL Shape Files (*.shape)|*.shape|All Files (*.*)|*.*||";
	CFileDialog fileDlg( TRUE, ".shape", "*.shape", OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
	if (fileDlg.DoModal()==IDOK)
	{
		// Open a file
		CIFile file;
		CString fileName=fileDlg.GetPathName();
		if (file.open ((const char*)fileName))
		{
			// Sream a shape
			CShapeStream streamShape;
			try
			{
				// Stream it
				streamShape.serial (file);

				// Store the shape pointer
				ObjView->_ListShape.push_back (streamShape.getShapePointer());

				// Create a model and add it to the scene
				CTransformShape	*pTrShape=streamShape.getShapePointer()->createInstance(CNELU::Scene);

				// Store the transform shape pointer
				ObjView->_ListTransformShape.push_back (pTrShape);

				// Reset the camera
				OnResetCamera();
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
			_snprintf (msg, 512, "Can't open the file %s for reading.", fileDlg.GetPathName());
			MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
		}
	}
}

void CSceneDlg::OnLoadPlaylist() 
{
	// TODO: Add your control notification handler code here
	
}

void CSceneDlg::OnLoadScene() 
{
	// TODO: Add your control notification handler code here
	
}

void CSceneDlg::OnSavePlaylist() 
{
	// TODO: Add your control notification handler code here
	
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
	if (ObjView->_ListTransformShape.size())
	{
		// Pointer on the CMesh;
		CTransformShape		*pTransform=ObjView->_ListTransformShape[0];
		IShape*				pShape=pTransform->Shape;
		CMesh				*pMesh=(CMesh*)pShape;

		// Reset the hotspot
		CVector hotSpot=pTransform->getMatrix()*pMesh->getBoundingBox().getCenter();

		// Reset the radius
		float radius=pMesh->getBoundingBox().getRadius();

		// For each model in the list
		for (uint m=1; m<ObjView->_ListTransformShape.size(); m++)
		{
			// Pointer on the CMesh;
			pTransform=ObjView->_ListTransformShape[m];
			pShape=pTransform->Shape;
			pMesh=(CMesh*)pShape;

			// Get the hotspot
			CVector hotSpot2=pTransform->getMatrix()*pMesh->getBoundingBox().getCenter();

			// Get the radius
			float radius2=pMesh->getBoundingBox().getRadius();

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

		// Setup camera
		CNELU::Camera->lookAt (hotSpot+CVector(0.57735f,0.57735f,0.57735f)*radius, hotSpot);

		// Setup mouse listener
		ObjView->_MouseListener.setMatrix (CNELU::Camera->getMatrix());
		ObjView->_MouseListener.setFrustrum (CNELU::Camera->getFrustum());
		ObjView->_MouseListener.setViewport (CViewport());
		ObjView->_MouseListener.setHotSpot (hotSpot);
		ObjView->_MouseListener.setMouseMode (CEvent3dMouseListener::edit3d);
	}
	else
	{
		// Setup camera
		CNELU::Camera->lookAt (CVector(1,1,1), CVector(0,0,0));

		// Setup mouse listener
		ObjView->_MouseListener.setMatrix (CNELU::Camera->getMatrix());
		ObjView->_MouseListener.setFrustrum (CNELU::Camera->getFrustum());
		ObjView->_MouseListener.setViewport (CViewport());
		ObjView->_MouseListener.setHotSpot (CVector(0,0,0));
		ObjView->_MouseListener.setMouseMode (CEvent3dMouseListener::edit3d);
	}
}
