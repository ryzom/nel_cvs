// object_viewer.h : main header file for the OBJECT_VIEWER DLL
//

#ifndef OBJECT_VIEWER_INTERFACE
#define OBJECT_VIEWER_INTERFACE

#ifndef OBJECT_VIEWER_EXPORT
#define OBJECT_VIEWER_EXPORT __declspec( dllimport ) 
#endif // OBJECT_VIEWER_EXPORT

// Increment this version number each time you distribute a new version of the dll.
#define OBJECT_VIEWER_VERSION 1

namespace NL3D
{
	class IShape;
	class CAnimation;
	class CLight;
}

namespace NLMISC
{
	class CRGBA;
}

class IObjectViewer
{
public:
	virtual ~IObjectViewer () {};

	// Init the UI
	virtual void initUI ()=0;

	// Go
	virtual void go ()=0;

	// Release the UI
	virtual void releaseUI ()=0;

	// Add a shape
	virtual void addShape (NL3D::IShape*	pShape, const char* name)=0;

	// Load a shape
	virtual bool loadShape (const char* filename)=0;

	// Load a shape
	virtual void resetCamera ()=0;

	// Set single animation
	virtual void setSingleAnimation (NL3D::CAnimation* pAnim, const char* name)=0;

	// Set ambient color
	virtual void setAmbientColor (const NLMISC::CRGBA& color)=0;

	// Set ambient color
	virtual void setLight (unsigned char id, const NL3D::CLight& light)=0;

	// Get instance
	static OBJECT_VIEWER_EXPORT IObjectViewer* getInterface (int version=OBJECT_VIEWER_VERSION);

	// Release instance
	static OBJECT_VIEWER_EXPORT void releaseInterface (IObjectViewer* view);
};

#endif OBJECT_VIEWER_INTERFACE
