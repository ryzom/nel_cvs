// animation_set_dlg.cpp : implementation file
//

#include "std_afx.h"
#include "object_viewer.h"
#include "animation_set_dlg.h"
#include <nel/misc/file.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace NLMISC;
using namespace NL3D;

/////////////////////////////////////////////////////////////////////////////
// CAnimationSetDlg dialog


CAnimationSetDlg::CAnimationSetDlg(CObjectViewer* objView, CWnd* pParent /*=NULL*/)
	: CDialog(CAnimationSetDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnimationSetDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT

	_ObjView=objView;
}


void CAnimationSetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnimationSetDlg)
	DDX_Control(pDX, IDC_TREE, Tree);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAnimationSetDlg, CDialog)
	//{{AFX_MSG_MAP(CAnimationSetDlg)
	ON_BN_CLICKED(IDC_ADD_ANIMATION, OnAddAnimation)
	ON_BN_CLICKED(IDC_RESET, OnReset)
	ON_BN_CLICKED(IDC_ADD_SKEL_WT, OnAddSkelWt)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnimationSetDlg message handlers

// ***************************************************************************

void CAnimationSetDlg::OnAddAnimation () 
{
	// TODO: Add your control notification handler code here
	static char BASED_CODE szFilter[] = "NeL Animation Files (*.anim)|*.anim|All Files (*.*)|*.*||";
	CFileDialog fileDlg( TRUE, ".anim", "*.anim", OFN_ALLOWMULTISELECT|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
	if (fileDlg.DoModal()==IDOK)
	{
		// Open the file
		try
		{
			// Get first file
			POSITION pos=fileDlg.GetStartPosition( );
			while (pos)
			{
				// Get the name
				CString filename=fileDlg.GetNextPathName(pos);

				// Load the animation
				loadAnimation (filename);

				// Touch the channel mixer
				_ObjView->reinitChannels ();
			}
		}
		catch (Exception& e)
		{
			MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
		}
	}
}

// ***************************************************************************

void CAnimationSetDlg::OnAddSkelWt() 
{
	// TODO: Add your control notification handler code here
	static char BASED_CODE szFilter[] = "NeL Skeleton Weight Template Files (*.skwt)|*.skwt|All Files (*.*)|*.*||";
	CFileDialog fileDlg( TRUE, ".skwt", "*.skwt", OFN_ALLOWMULTISELECT|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT, szFilter);
	if (fileDlg.DoModal()==IDOK)
	{
		// Open the file
		try
		{
			// Get first file
			POSITION pos=fileDlg.GetStartPosition( );
			while (pos)
			{
				// Get the name
				CString filename=fileDlg.GetNextPathName(pos);

				// Load the animation
				loadSkeleton (filename);

				// Touch the channel mixer
				_ObjView->reinitChannels  ();
			}
		}
		catch (Exception& e)
		{
			MessageBox (e.what(), "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
		}
	}
}

// ***************************************************************************

void CAnimationSetDlg::OnReset () 
{
	// Reset the channel mixer slots
	_ObjView->resetSlots ();

	// Clear the filename array
	_ListAnimation.clear();
	_ListSkeleton.clear();

	// Clear the TREE
	Tree.DeleteAllItems ();
}

// ***************************************************************************

void CAnimationSetDlg::loadAnimation (const char* fileName)
{
	// Open the file
	CIFile file;
	if (file.open (fileName))
	{
		// Get the animation name
		char name[256];
		_splitpath (fileName, NULL, NULL, name, NULL);

		// Make an animation
		CAnimation *anim=new CAnimation;

		// Serial it
		anim->serial (file);

		// Add the animation
		addAnimation (anim, name);

		// Add the filename in the list
		_ListAnimation.push_back (fileName);
	}
	else
	{
		// Create a message
		char msg[512];
		_snprintf (msg, 512, "Can't open the file %s for reading.", fileName);
		MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
	}

}

// ***************************************************************************

void CAnimationSetDlg::addAnimation (NL3D::CAnimation* anim, const char* name)
{
	// Add an animation
	_ObjView->_AnimationSet.addAnimation (name, anim);

	// Rebuild the animationSet
	_ObjView->_AnimationSet.build ();

	// Insert an intem
	HTREEITEM item=Tree.InsertItem (name);
	nlassert (item!=NULL);

	// For all tracks in the animation
	std::set<std::string> setString;
	anim->getTrackNames (setString);
	std::set<std::string>::iterator ite=setString.begin();
	while (ite!=setString.end())
	{
		// Add this string
		Tree.InsertItem (ite->c_str(), item);

		ite++;
	}
}

// ***************************************************************************

void CAnimationSetDlg::loadSkeleton (const char* fileName)
{
	// Open the file
	CIFile file;
	if (file.open (fileName))
	{
		// Get the animation name
		char name[256];
		_splitpath (fileName, NULL, NULL, name, NULL);

		// Make an animation
		CAnimation *anim=new CAnimation;

		// Add an animation
		uint id=_ObjView->_AnimationSet.addSkeletonWeight (name);

		// Get the skeleton pointer
		CSkeletonWeight* skel=_ObjView->_AnimationSet.getSkeletonWeight (id);

		// Serial it
		skel->serial (file);

		// Add the filename in the list
		_ListSkeleton.push_back (fileName);

		// Insert an intem
		HTREEITEM item=Tree.InsertItem (name);
		nlassert (item!=NULL);

		// Get number of node in this skeleton weight
		uint numNode=skel->getNumNode ();

		// Add the nodein the tree
		for (uint n=0; n<numNode; n++)
		{
			// Add this string
			Tree.InsertItem (skel->getNodeName (n).c_str(), item);
		}
	}
	else
	{
		// Create a message
		char msg[512];
		_snprintf (msg, 512, "Can't open the file %s for reading.", fileName);
		MessageBox (msg, "NeL object viewer", MB_OK|MB_ICONEXCLAMATION);
	}

}
