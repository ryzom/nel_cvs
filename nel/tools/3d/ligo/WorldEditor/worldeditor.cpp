/** \file WorldEditor.cpp
 * : Defines the initialization routines for the DLL.
 *
 * $Id: worldeditor.cpp,v 1.3 2001/12/05 16:43:38 besson Exp $
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





#include "stdafx.h"

#undef WORLDEDITOR_EXPORT
#define WORLDEDITOR_EXPORT __declspec( dllexport ) 


#include "WorldEditor.h"
#include "mainfrm.h"
#include "resource.h"

#include "3d/nelu.h"
#include "3d/init_3d.h"

#include "nel/misc/vector.h"
#include "nel/misc/stream.h"
#include "nel/misc/o_xml.h"
#include "nel/misc/file.h"

#include "../lib/primitive.h"

#include <vector>
#include <string>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


static char SDrive[256];
static char SDir[256];



using namespace std;
using namespace NL3D;
using namespace NLMISC;
using namespace NLLIGO;

//
//	Note!
//
//		If this DLL is dynamically linked against the MFC
//		DLLs, any functions exported from this DLL which
//		call into MFC must have the AFX_MANAGE_STATE macro
//		added at the very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorApp
class CWorldEditorApp : public CWinApp
{
public:
	CWorldEditorApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CObject_viewerApp)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CObject_viewerApp)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


BEGIN_MESSAGE_MAP(CWorldEditorApp, CWinApp)
	//{{AFX_MSG_MAP(CWorldEditorApp)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorldEditorApp construction

CWorldEditorApp::CWorldEditorApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CWorldEditorApp object

CWorldEditorApp theApp;

// ***************************************************************************

CWorldEditor::CWorldEditor ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
//	init3d ();
	_MainFrame = NULL;
	_RootDir = "";
}

// ***************************************************************************

CWorldEditor::~CWorldEditor ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (_MainFrame)
		delete (_MainFrame);
}

// ***************************************************************************

void *CWorldEditor::getMainFrame ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (_MainFrame == NULL)
		initUI();
	return _MainFrame;
}

// ***************************************************************************

void CWorldEditor::setRootDir (const char *sPathName)
{
	_RootDir = sPathName;
}

// ***************************************************************************

void CWorldEditor::createDefaultFiles (const char *fileBaseName)
{
	// The primitive region
	CPrimRegion reg;
	string fileName = fileBaseName;
	fileName += ".prim";
	reg.Name = fileName;
	COFile file;
	file.open (fileName);
	COXml output;
	output.init (&file, "1.0");
	reg.serial (output);
	output.flush ();
	file.close ();

	// The landscape region
	CBuilderZoneRegion bzr;
	fileName = fileBaseName;
	fileName += ".land";
	file.open (fileName);
	bzr.serial (file);
	file.close();
}

// ***************************************************************************

void CWorldEditor::loadFile(const char *fileName)
{
	char sCurDir[MAX_PATH];
	GetCurrentDirectory (MAX_PATH, sCurDir);

	char drive[_MAX_DRIVE];
	char dir[_MAX_DIR];
	char fname[_MAX_FNAME];
	char ext[_MAX_EXT];

	_splitpath(fileName, drive, dir, fname, ext);

	string sPath;
	string sTemp;
	sPath = drive;
	sPath += dir;
	SetCurrentDirectory (sPath.c_str());
	if (stricmp(ext, ".prim") == 0)
	{
		sTemp = fname;
		sTemp += ".prim";
		_MainFrame->loadPrim (sTemp.c_str(), sPath.c_str());
	}
	if (stricmp(ext, ".land") == 0)
	{
		sTemp = fname;
		sTemp += ".land";
		_MainFrame->loadLand (sTemp.c_str(), sPath.c_str());
	}
	SetCurrentDirectory (sCurDir);
}

// ***************************************************************************

void CWorldEditor::saveOpenedFiles()
{
	_MainFrame->saveAll ();
}

// ***************************************************************************

void CWorldEditor::initUI (HWND parent)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	if (_MainFrame != NULL)
		return;

	_MainFrame = new CMainFrame();

	BOOL bRet = _MainFrame->LoadFrame (IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);

	_MainFrame->setRootDir (_RootDir.c_str());
	_MainFrame->ShowWindow (SW_SHOW);
	_MainFrame->UpdateWindow ();
	_MainFrame->init ();
	_MainFrame->initDisplay ();
	_MainFrame->initTools ();
}

// ***************************************************************************

void CWorldEditor::initUILight (int x, int y, int cx, int cy)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	if (_MainFrame != NULL)
		return;

	_MainFrame = new CMainFrame();

	_MainFrame->createX = x;
	_MainFrame->createY = y;
	_MainFrame->createCX = cx;
	_MainFrame->createCY = cy;
	BOOL bRet = _MainFrame->LoadFrame (IDR_MAINFRAME, WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, NULL, NULL);
	CMenu* pMenu = _MainFrame->GetMenu();

	if (pMenu != NULL && pMenu->GetMenuItemCount() > 0)
	{
		pMenu->DeleteMenu(0, MF_BYPOSITION);
		// force a redraw of the menu bar
		_MainFrame->DrawMenuBar();
	}
	_MainFrame->setRootDir (_RootDir.c_str());
	_MainFrame->ShowWindow (SW_SHOW);
	_MainFrame->UpdateWindow ();
	_MainFrame->init (false);
	_MainFrame->initDisplay ();
	_MainFrame->initTools ();
}

// ***************************************************************************

void CWorldEditor::go ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	_MainFrame->_Exit = false;
	do
	{
		// Pump message from the server
//		CNELU::EventServer.pump();

		// Pump others message for the windows
		MSG	msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	while (!_MainFrame->_Exit);
}

// ***************************************************************************

void CWorldEditor::releaseUI ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	// exit
	delete _MainFrame;
	_MainFrame = NULL;
	CNELU::release();
}

// ***************************************************************************

void setRegisterWindowState (const CWnd *pWnd, const char* keyName)
{
	HKEY hKey;
	if (RegCreateKey(HKEY_CURRENT_USER, keyName, &hKey)==ERROR_SUCCESS)
	{
		RECT rect;
		pWnd->GetWindowRect (&rect);
		RegSetValueEx(hKey, "Left", 0, REG_DWORD, (LPBYTE)&rect.left, 4);
		RegSetValueEx(hKey, "Right", 0, REG_DWORD, (LPBYTE)&rect.right, 4);
		RegSetValueEx(hKey, "Top", 0, REG_DWORD, (LPBYTE)&rect.top, 4);
		RegSetValueEx(hKey, "Bottom", 0, REG_DWORD, (LPBYTE)&rect.bottom, 4);
	}
}

// ***************************************************************************

void getRegisterWindowState (CWnd *pWnd, const char* keyName, bool resize)
{
	HKEY hKey;
	if (RegOpenKeyEx(HKEY_CURRENT_USER, keyName, 0, KEY_READ, &hKey)==ERROR_SUCCESS)
	{
		DWORD len=4;
		DWORD type;
		RECT rect;
		RegQueryValueEx (hKey, "Left", 0, &type, (LPBYTE)&rect.left, &len);
		RegQueryValueEx (hKey, "Right", 0, &type, (LPBYTE)&rect.right, &len);
		RegQueryValueEx (hKey, "Top", 0, &type, (LPBYTE)&rect.top, &len);
		RegQueryValueEx (hKey, "Bottom", 0, &type, (LPBYTE)&rect.bottom, &len);

		// Set window pos
		pWnd->SetWindowPos (NULL, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_NOOWNERZORDER|SWP_NOZORDER|
			(resize?0:SWP_NOSIZE));
	}
}

// ***************************************************************************
// ***************************************************************************
// WorldEditor Object Factory
// ***************************************************************************
// ***************************************************************************

IWorldEditor* IWorldEditor::getInterface (int version)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Check version number
	if (version != WORLDEDITOR_VERSION)
	{
		MessageBox (NULL, "Bad version of WorldEditor.dll.", "NeL world editor", MB_ICONEXCLAMATION|MB_OK);
		return NULL;
	}
	else
		return new CWorldEditor;
}

// ***************************************************************************

void IWorldEditor::releaseInterface (IWorldEditor* wed)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	delete wed;
}

// ***************************************************************************

IWorldEditor* IWorldEditorGetInterface (int version)
{
	return IWorldEditor::getInterface (version);
}

// ***************************************************************************

void IWorldEditorReleaseInterface (IWorldEditor* pWE)
{
	IWorldEditor::releaseInterface (pWE);
}