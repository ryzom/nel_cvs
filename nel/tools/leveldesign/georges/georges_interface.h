
#ifndef GEORGES_INTERFACE
#define GEORGES_INTERFACE

#ifndef GEORGES_EXPORT
#define GEORGES_EXPORT __declspec( dllimport ) 
#endif // GEORGES_EXPORT

// Increment this version number each time you distribute a new version of the dll.
#define GEORGES_VERSION 1

/**
 * IGeorges
 *
 * \author Matthieu "TrapII" Besson
 * \author Nevrax France
 * \date 2001
 */
class IGeorges
{
public:
	virtual ~IGeorges() {};

	// Init the UI
	virtual void initUI (HWND parent=NULL)=0;

	// Init the UI Light version
	virtual void initUILight (int x, int y, int cx, int cy)=0;

	// Go
	virtual void go ()=0;

	// Release the UI
	virtual void releaseUI ()=0;

	// Get the main frame
	virtual void*getMainFrame ()=0;
	
	// Get instance
	static GEORGES_EXPORT IGeorges * getInterface (int version = GEORGES_VERSION);

	// Release instance
	static GEORGES_EXPORT void releaseInterface (IGeorges* pGeorges);
};


// To export the names in a good format that can be human readable and not with the heavy style
// of the MFC we have to do it in 'old-school' mode
extern "C" 
{
	GEORGES_EXPORT IGeorges* IGeorgesGetInterface (int version = GEORGES_VERSION);
	GEORGES_EXPORT void IGeorgesReleaseInterface (IGeorges * pG);
}

#endif LOGIC_EDITOR_INTERFACE
