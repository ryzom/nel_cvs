// edit_morph_mesh_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "edit_morph_mesh_dlg.h"
#include "3d/ps_mesh.h"
#include "3d/particle_system_model.h"

#include "attrib_dlg.h"
#include "particle_dlg.h"


/////////////////////////////////////////////////////////////////////////////
// CEditMorphMeshDlg dialog


CEditMorphMeshDlg::CEditMorphMeshDlg(NL3D::CPSConstraintMesh *cm, CWnd* pParent, CParticleDlg  *particleDlg, IPopupNotify *pn /*= NULL*/)
	: _PN(pn), _CM(cm), CDialog(CEditMorphMeshDlg::IDD, pParent), _ParticleDlg(particleDlg)
{
	nlassert(cm);
	//{{AFX_DATA_INIT(CEditMorphMeshDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CEditMorphMeshDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CEditMorphMeshDlg)
	DDX_Control(pDX, IDC_MESHS, m_MeshList);
	//}}AFX_DATA_MAP
}



BEGIN_MESSAGE_MAP(CEditMorphMeshDlg, CDialog)
	//{{AFX_MSG_MAP(CEditMorphMeshDlg)
	ON_BN_CLICKED(IDC_ADD, OnAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnRemove)
	ON_BN_CLICKED(IDC_CHANGE, OnChange)
	ON_WM_CLOSE()
	ON_BN_CLICKED(IDC_INSERT, OnInsert)
	ON_BN_CLICKED(IDC_UP, OnUp)
	ON_BN_CLICKED(IDC_DOWN, OnDown)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

//====================================================================
void CEditMorphMeshDlg::init(CWnd *pParent)
{
	Create(IDD_EDIT_MORPH_MESH, pParent);	
	ShowWindow(SW_SHOW);
}


//====================================================================
bool CEditMorphMeshDlg::getShapeNameFromDlg(std::string &name)
{
	CFileDialog fd(TRUE, ".shape", "*.shape", 0, NULL, this);
	if (fd.DoModal() == IDOK)
	{
		// Add to the path
		char drive[256];
		char dir[256];
		char path[256];
		char fname[256];
		char ext[256];


		// Add search path for the texture
		_splitpath (fd.GetPathName(), drive, dir, fname, ext);
		_makepath (path, drive, dir, NULL, NULL);
		NLMISC::CPath::addSearchPath (path);

		name = std::string(fname) + std::string(ext);
		
		return true;
	}
	else
	{
		return false;
	}
}

//====================================================================
void CEditMorphMeshDlg::OnAdd() 
{
	std::string shapeName;
	if (getShapeNameFromDlg(shapeName))
	{		
		std::vector<std::string> shapeNames;
		shapeNames.resize(_CM->getNumShapes() + 1);
		_CM->getShapesNames(&shapeNames[0]);
		shapeNames[shapeNames.size() - 1] = shapeName;
		_CM->setShapes(&shapeNames[0], shapeNames.size());
		m_MeshList.AddString(shapeName.c_str());
		GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);
	}
	_ParticleDlg->getCurrPSModel()->touchTransparencyState();
}

//====================================================================
void CEditMorphMeshDlg::OnRemove() 
{
	UpdateData();
	sint selItem = m_MeshList.GetCurSel();
	std::vector<std::string> shapeNames;
	shapeNames.resize(_CM->getNumShapes());
	_CM->getShapesNames(&shapeNames[0]);
	shapeNames.erase(shapeNames.begin() + selItem);
	_CM->setShapes(&shapeNames[0], shapeNames.size());
	if (_CM->getNumShapes() == 2)
	{
		GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
	}
	_ParticleDlg->getCurrPSModel()->touchTransparencyState();
	updateMeshList();
}

//====================================================================
void CEditMorphMeshDlg::OnInsert() 
{
	std::string shapeName;
	if (getShapeNameFromDlg(shapeName))
	{	
		sint selItem = m_MeshList.GetCurSel();
		std::vector<std::string> shapeNames;
		shapeNames.resize(_CM->getNumShapes());
		_CM->getShapesNames(&shapeNames[0]);
		shapeNames.insert(shapeNames.begin() + selItem, shapeName);
		_CM->setShapes(&shapeNames[0], shapeNames.size());		
		GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);
		_ParticleDlg->getCurrPSModel()->touchTransparencyState();
		updateMeshList();
		m_MeshList.SetCurSel(selItem);
	}	
}

//====================================================================
void CEditMorphMeshDlg::OnUp() 
{		
	sint selItem = m_MeshList.GetCurSel();
	if (selItem == 0) return;
	std::vector<std::string> shapeNames;
	shapeNames.resize(_CM->getNumShapes());
	_CM->getShapesNames(&shapeNames[0]);
	std::swap(shapeNames[selItem - 1], shapeNames[selItem]);
	_CM->setShapes(&shapeNames[0], shapeNames.size());		
	GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);		
	updateMeshList();
	m_MeshList.SetCurSel(selItem - 1);	
}

//====================================================================
void CEditMorphMeshDlg::OnDown() 
{
	sint selItem = m_MeshList.GetCurSel();
	if (selItem == (sint) (_CM->getNumShapes() - 1)) return;
	std::vector<std::string> shapeNames;
	shapeNames.resize(_CM->getNumShapes());
	_CM->getShapesNames(&shapeNames[0]);
	std::swap(shapeNames[selItem + 1], shapeNames[selItem]);
	_CM->setShapes(&shapeNames[0], shapeNames.size());		
	GetDlgItem(IDC_REMOVE)->EnableWindow(TRUE);		
	updateMeshList();
	m_MeshList.SetCurSel(selItem + 1);	
}

//====================================================================
void CEditMorphMeshDlg::OnChange() 
{
	UpdateData();
	std::string shapeName;
	if (getShapeNameFromDlg(shapeName))
	{			
		sint selItem = m_MeshList.GetCurSel();
		_CM->setShape(selItem, shapeName);	
		updateMeshList();
		_ParticleDlg->getCurrPSModel()->touchTransparencyState();
	}
}

//====================================================================
float CEditMorphMeshDlg::CMorphSchemeWrapper::get(void) const
{
	nlassert(CM);
	return CM->getMorphValue();
}	

//====================================================================	
void CEditMorphMeshDlg::CMorphSchemeWrapper::set(const float &v)
{
	nlassert(CM);
	CM->setMorphValue(v);
}

//====================================================================
CEditMorphMeshDlg::CMorphSchemeWrapper::scheme_type *CEditMorphMeshDlg::CMorphSchemeWrapper::getScheme(void) const
{
	nlassert(CM);
	return CM->getMorphScheme();
}

//====================================================================
void CEditMorphMeshDlg::CMorphSchemeWrapper::setScheme(scheme_type *s)
{
	nlassert(CM);
	CM->setMorphScheme(s);
}

//====================================================================
void CEditMorphMeshDlg::updateMeshList()
{
	m_MeshList.ResetContent();
	for (uint k = 0; k < _CM->getNumShapes(); ++k)
	{
		m_MeshList.AddString(_CM->getShape(k).c_str());
	}
	m_MeshList.SetCurSel(0);
	UpdateData(FALSE);
}

//====================================================================
BOOL CEditMorphMeshDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	updateMeshList(); // fill the list box with the neames of the meshs

	/// create the morph scheme edition dialog
	RECT r;
	CAttribDlgFloat *mvd = new CAttribDlgFloat("MORPH_VALUE");
	_MorphSchemeWrapper.CM = _CM;
	mvd->setWrapper(&_MorphSchemeWrapper);	
	mvd->setSchemeWrapper(&_MorphSchemeWrapper);
	GetDlgItem(IDC_MORPH_SCHEME)->GetWindowRect(&r);
	ScreenToClient(&r);
	HBITMAP bmh = LoadBitmap(::AfxGetInstanceHandle(), MAKEINTRESOURCE(IDB_MORPH_SCHEME));
	mvd->init(bmh, r.left, r.top, this);	
	pushWnd(mvd);
	if (_CM->getNumShapes() == 2)
	{
		GetDlgItem(IDC_REMOVE)->EnableWindow(FALSE);
	}	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


//====================================================================
void CEditMorphMeshDlg::OnClose() 
{
	CDialog::OnClose();
	if (_PN) _PN->childPopupClosed(this);	
}

