// Interface Implementation

#include "stdafx.h"

#undef GEORGES_EXPORT
#define GEORGES_EXPORT __declspec( dllexport ) 

#include "georges_interface.h"
#include "georges.h"
#include "Mainfrm.h"


/////////////////////////////////////////////////////////////////////////////
// The one and only CGeorgesApp object

CGeorgesApp theApp;



// The interface implemented through this class
class CGeorgesImpl : public IGeorges
{
	CMainFrame *_MainFrame;

public:

	CGeorgesImpl();

	virtual ~CGeorgesImpl();

	// Init the UI
	virtual void initUI (HWND parent=NULL);

	// Init the UI Light version
	virtual void initUILight (int x, int y, int cx, int cy);

	// Go
	virtual void go ();

	// Release the UI
	virtual void releaseUI ();

	// Get the main frame
	virtual void*getMainFrame ();
	
	// Get instance
	static GEORGES_EXPORT IGeorges * getInterface (int version = GEORGES_VERSION);

	// Release instance
	static GEORGES_EXPORT void releaseInterface (IGeorges* pGeorges);
};

//---------------------------------------------
//	CGeorgesImpl
//
//---------------------------------------------
CGeorgesImpl::CGeorgesImpl()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());	
	_MainFrame = NULL;
	
} // CGeorgesImpl //


//---------------------------------------------
//	CGeorgesImpl
//
//---------------------------------------------
CGeorgesImpl::~CGeorgesImpl()
{
}


//---------------------------------------------
//	initUI
//
//---------------------------------------------
void CGeorgesImpl::initUI( HWND parent )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	theApp.initInstance();

	_MainFrame = (CMainFrame*)theApp.m_pMainWnd;
	
} // initUI //



//---------------------------------------------
//	initUILight
//
//---------------------------------------------
void CGeorgesImpl::initUILight (int x, int y, int cx, int cy)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	theApp.initInstance (x, y, cx, cy);
		
	// The main window has been initialized, so show and update it.
	_MainFrame = (CMainFrame*)theApp.m_pMainWnd;

} // initUILight //



//---------------------------------------------
//	Go
//
//---------------------------------------------
void CGeorgesImpl::go()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	do
	{
		MSG	msg;
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	while (!_MainFrame->Exit);

} // go



//---------------------------------------------
//	releaseUI
//
//---------------------------------------------
void CGeorgesImpl::releaseUI()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	delete _MainFrame;
	_MainFrame = NULL;
} // releaseUI




//---------------------------------------------
//	getMainFrame
//
//---------------------------------------------
void * CGeorgesImpl::getMainFrame ()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	
	if (theApp.m_pMainWnd == NULL)
		initUI();
	
	return theApp.m_pMainWnd;

} // getMainFrame //

// *******
// STATICS
// *******

//---------------------------------------------
//	releaseInterface
//
//---------------------------------------------
void IGeorges::releaseInterface (IGeorges* pGeorges)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	delete pGeorges;

} // releaseInterface //

//---------------------------------------------
//	getInterface
//
//---------------------------------------------
IGeorges* IGeorges::getInterface (int version)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

	// Check version number
	if (version != GEORGES_VERSION)
	{
		MessageBox( NULL, "Bad version of georges.dll.", "Georges", MB_ICONEXCLAMATION|MB_OK);
		return NULL;
	}
	else
		return new CGeorgesImpl;

} // getInterface //



//---------------------------------------------
//	IGeorgesGetInterface (Helper name)
//---------------------------------------------
IGeorges* IGeorgesGetInterface (int version)
{
	return IGeorges::getInterface (version);

} // IGeorgesGetInterface //


//---------------------------------------------
//	ILogicEditorReleaseInterface
//---------------------------------------------
void IGeorgesReleaseInterface (IGeorges* pGeorges)
{
	IGeorges::releaseInterface (pGeorges);

} // IGeorgesReleaseInterface //


