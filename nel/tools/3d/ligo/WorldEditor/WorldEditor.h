// WorldEditor.h : main header file for the WORLDEDITOR application
//

#include "worldeditor_interface.h"

class CMainFrame;

class CWorldEditor : public IWorldEditor
{

	CMainFrame *_MainFrame;

public:
	
	CWorldEditor();

	virtual ~CWorldEditor();

	// Init the UI
	virtual void initUI (HWND parent=NULL);

	// Go
	virtual void go ();

	// Release the UI
	virtual void releaseUI ();

	virtual void*getMainFrame ();
};