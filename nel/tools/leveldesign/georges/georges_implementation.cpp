// Interface Implementation

#include "stdafx.h"

#undef GEORGES_EXPORT
#define GEORGES_EXPORT __declspec( dllexport ) 

#include "georges_interface.h"
#include "georges.h"
#include "Mainfrm.h"
#include "GeorgesDoc.h"
//#include "../georges_lib/Loader.h"


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
	virtual void* getMainFrame (); 
	
	// Get instance
	static GEORGES_EXPORT IGeorges* getInterface (int version = GEORGES_VERSION); 

	// Release instance
	static GEORGES_EXPORT void releaseInterface (IGeorges* pGeorges);


	virtual void SetDocumentWorkDirectory( const std::string& _sxworkdirectory );

	virtual void SetDocumentRootDirectory( const std::string& _sxrootdirectory );

	virtual void NewDocument();

	virtual void NewDocument( const std::string& _sxdfnname);

	virtual void LoadDocument( const std::string& _sxfullname );

	virtual void SaveDocument( const std::string& _sxfullname );

	virtual void CloseDocument();


	virtual void SetWorkDirectory( const std::string& _sxworkdirectory );

	virtual void SetRootDirectory( const std::string& _sxrootdirectory );

	virtual void SaveAllDocument();

	virtual void CloseAllDocument();

	virtual void SetTypPredef( const std::string& _sxfilename, const std::vector< std::string >& _pvsx );

	virtual void MakeDfn( const std::string& _sxfullname, const std::vector< std::pair< std::string, std::string > >* const _pvdefine = 0 );

	virtual void MakeTyp( const std::string& _sxfullname, const std::string& _sxtype, const std::string& _sxformula, const std::string& _sxenum, const std::string& _sxlow, const std::string& _sxhigh, const std::string& _sxdefault, const std::vector< std::pair< std::string, std::string > >* const _pvpredef = 0, const std::vector< std::pair< std::string, std::string > >* const _pvparent = 0 );
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
	_MainFrame->DestroyWindow();
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

void CGeorgesImpl::SetWorkDirectory( const std::string& _sxworkdirectory )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesApp* papp = dynamic_cast< CGeorgesApp* >( AfxGetApp() );
	papp->SetWorkDirectory( _sxworkdirectory );
} // SetWorkDirectory //

void CGeorgesImpl::SetRootDirectory( const std::string& _sxrootdirectory )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesApp* papp = dynamic_cast< CGeorgesApp* >( AfxGetApp() );
	papp->SetRootDirectory( _sxrootdirectory );
} // SetRootDirectory //

void CGeorgesImpl::SetTypPredef( const std::string& _sxfilename, const std::vector< std::string >& _pvs )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesApp* papp = dynamic_cast< CGeorgesApp* >( AfxGetApp() );
	CLoader* ploader = papp->GetLoader();
	std::vector< CStringEx > vsx;
	for( std::vector< std::string >::const_iterator it = _pvs.begin(); it != _pvs.end(); ++it )
		vsx.push_back( *it );
	ploader->SetTypPredef( _sxfilename, vsx );
} // SetTypPredef //

void CGeorgesImpl::MakeDfn( const std::string& _sxfullname, const std::vector< std::pair< std::string, std::string > >* const _pvdefine  )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesApp* papp = dynamic_cast< CGeorgesApp* >( AfxGetApp() );
	CLoader* ploader = papp->GetLoader();
	std::vector< std::pair< CStringEx, CStringEx > >* vsx = 0;
	if( _pvdefine )
	{
		std::vector< std::pair< CStringEx, CStringEx > > v;
		vsx = &v;
		for( std::vector< std::pair< std::string, std::string > >::const_iterator it = _pvdefine->begin(); it != _pvdefine->end(); ++it )
			vsx->push_back( std::make_pair( it->first , it->second  ) );
	}
	ploader->MakeDfn( _sxfullname, vsx );
} // MakeDfn //

void CGeorgesImpl::MakeTyp( const std::string& _sxfullname, const std::string& _sxtype, const std::string& _sxformula, const std::string& _sxenum, const std::string& _sxlow, const std::string& _sxhigh, const std::string& _sxdefault, const std::vector< std::pair< std::string, std::string > >* const _pvpredef , const std::vector< std::pair< std::string, std::string > >* const _pvparent  )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesApp* papp = dynamic_cast< CGeorgesApp* >( AfxGetApp() );
	CLoader* ploader = papp->GetLoader();
	std::vector< std::pair< CStringEx, CStringEx > >* vsx = 0;
	std::vector< std::pair< CStringEx, CStringEx > >* vsx2 = 0;
	if( _pvpredef )
	{
		std::vector< std::pair< CStringEx, CStringEx > > v; 
		vsx = &v;
		for( std::vector< std::pair< std::string, std::string > >::const_iterator it = _pvpredef->begin(); it != _pvpredef->end(); ++it )
			vsx->push_back( std::make_pair( it->first, it->second ) );
	}
	if( _pvparent )
	{
		std::vector< std::pair< CStringEx, CStringEx > > v2;
		vsx2 = &v2;
		for( std::vector< std::pair< std::string, std::string > >::const_iterator it = _pvparent->begin(); it != _pvparent->end(); ++it )
			vsx2->push_back( std::make_pair( it->first, it->second ) );
	}
	ploader->MakeTyp( _sxfullname, _sxtype, _sxformula, _sxenum, _sxlow, _sxhigh, _sxdefault, vsx, vsx2 );
} // MakeTyp //

void CGeorgesImpl::SetDocumentWorkDirectory( const std::string& _sxworkdirectory )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesDoc* pgdoc = dynamic_cast< CGeorgesDoc* >( ( (CMainFrame*)theApp.m_pMainWnd )->GetActiveDocument() );
	pgdoc->SetWorkDirectory( _sxworkdirectory );
} // SetDocumentWorkDirectory //

void CGeorgesImpl::SetDocumentRootDirectory( const std::string& _sxrootdirectory )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesDoc* pgdoc = dynamic_cast< CGeorgesDoc* >( ( (CMainFrame*)theApp.m_pMainWnd )->GetActiveDocument() );
	pgdoc->SetRootDirectory( _sxrootdirectory );
} // SetDocumentRootDirectory //

void CGeorgesImpl::LoadDocument( const std::string& _sxfullname )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesDoc* pgdoc = dynamic_cast< CGeorgesDoc* >( ( (CMainFrame*)theApp.m_pMainWnd )->GetActiveDocument() );
	pgdoc->OnOpenDocument( _sxfullname.c_str() );
} // LoadDocument //

void CGeorgesImpl::SaveDocument( const std::string& _sxfullname )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesDoc* pgdoc = dynamic_cast< CGeorgesDoc* >( ( (CMainFrame*)theApp.m_pMainWnd )->GetActiveDocument() );
	pgdoc->OnSaveDocument( _sxfullname.c_str() );
} // SaveDocument //

void CGeorgesImpl::SaveAllDocument()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesApp* papp = dynamic_cast< CGeorgesApp* >( AfxGetApp() );
	papp->SaveAllDocument();
} // SetRootDirectory //

void CGeorgesImpl::NewDocument( const std::string& _sxdfnname)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesDoc* pgdoc = dynamic_cast< CGeorgesDoc* >( ( (CMainFrame*)theApp.m_pMainWnd )->GetActiveDocument() );
	pgdoc->NewDocument( _sxdfnname );
} // NewDocument //

void CGeorgesImpl::NewDocument()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesDoc* pgdoc = dynamic_cast< CGeorgesDoc* >( ( (CMainFrame*)theApp.m_pMainWnd )->GetActiveDocument() );
	pgdoc->OnNewDocument();
} // NewDocument //

void CGeorgesImpl::CloseDocument()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesDoc* pgdoc = dynamic_cast< CGeorgesDoc* >( ( (CMainFrame*)theApp.m_pMainWnd )->GetActiveDocument() );
	pgdoc->OnCloseDocument();
} // CloseDocument //

void CGeorgesImpl::CloseAllDocument()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CGeorgesApp* papp = dynamic_cast< CGeorgesApp* >( AfxGetApp() );
	papp->CloseAllDocument();
} // CloseAllDocument //

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




