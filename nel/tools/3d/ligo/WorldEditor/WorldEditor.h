// WorldEditor.h : main header file for the WORLDEDITOR application
//

#include "worldeditor_interface.h"
#include <string>

class CMainFrame;

class CWorldEditor : public IWorldEditor
{

	CMainFrame	*_MainFrame;
	std::string _RootDir;
	IMasterCB	*_MCB;

public:
	
	CWorldEditor();

	virtual ~CWorldEditor();

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

	// Set the root path directory
	virtual void setRootDir (const char *sPathName);

	// To communicate the list of primzone to the master
	virtual void setMasterCB (IMasterCB *pMCB);

	// Create the default files given the base name (add extension)
	virtual void createDefaultFiles(const char *fileBaseName);

	// Create a default .prim file
	virtual void createEmptyPrimFile (const char *fullName);

	// Load a specific file and make it by default
	virtual void loadFile(const char *fileName);

	// Save all files opened
	virtual void saveOpenedFiles();
};