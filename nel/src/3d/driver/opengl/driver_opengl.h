#ifndef RK_OPENGL_H
#define RK_OPENGL_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <gl/gl.h>
#include "nel/3d/driver.h"
#include "nel/misc/types_nl.h"
#include "nel/misc/matrix.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"
#include "nel/misc/event_emitter.h"

namespace NL3D
{

using NLMISC::CMatrix;
using NLMISC::CVector;

// --------------------------------------------------

class CTextureDrvInfosGL : public ITextureDrvInfos
{
private:
public:
		GLuint		ID;
};

// --------------------------------------------------

class CVBDrvInfosGL : public IVBDrvInfos
{
private:
public:
};

// --------------------------------------------------

class CShaderGL : public IShader
{
private:
public:
	GLenum		SrcBlend;
	GLenum		DstBlend;
};

// --------------------------------------------------

class CDriverGL : public IDriver
{
private:
	// Version of the driver. Not the interface version!! Increment when implementation of the driver change.
	static const uint32		ReleaseVersion;

#ifdef NL_OS_WINDOWS
	friend static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
#endif // NL_OS_WINDOWS
private:
#ifdef NL_OS_WINDOWS
	HWND					_hWnd;
	HDC						_hDC;
	PIXELFORMATDESCRIPTOR	_pfd;
    HGLRC					_hRC;
	NLMISC::CEventEmitterWin32	_EventEmitter;
	static uint				_Registered;
	DEVMODE					_OldScreenMode;
	bool					_FullScreen;
	

#endif
	CShaderGL				_Shader;

	bool					_setupVertexBuffer(CVertexBuffer& VB);

	CMatrix					_ViewMtx;


private:
	bool					activateTexture(uint stage, ITexture *tex);


public:
							CDriverGL();
	virtual					~CDriverGL() { release(); };

	virtual bool			init(void);

	virtual ModeList		enumModes();

	virtual bool			setDisplay(void* wnd, const GfxMode& mode);

	virtual bool			activate(void);

	virtual NLMISC::IEventEmitter*	getEventEmitter(void) { return&_EventEmitter; };

	virtual bool			clear2D(CRGBA rgba);

	virtual bool			clearZBuffer(float zval=1);

	virtual bool			setupTexture(ITexture& tex);

	virtual bool			setupMaterial(CMaterial& mat);

	virtual void			setFrustum(float left, float right, float bottom, float top, float znear, float zfar, bool perspective = true);

	virtual void			setupViewMatrix(const CMatrix& mtx);

	virtual void			setupModelMatrix(const CMatrix& mtx, uint8 n=0);

	virtual CMatrix			getViewMatrix(void) const;

	virtual bool			activeVertexBuffer(CVertexBuffer& VB);

	virtual bool			render(CPrimitiveBlock& PB, CMaterial& Mat);

	virtual bool			swapBuffers(void);

	virtual bool			release(void);

	virtual TMessageBoxId	systemMessageBox (const char* message, const char* title, TMessageBoxType type=okType, TMessageBoxIcon icon=noIcon);

	virtual void			setupViewport (const class CViewport& viewport);

	virtual uint32			getImplementationVersion () const
	{
		return ReleaseVersion;
	}

	virtual const char*		getDriverInformation ()
	{
		return "Opengl 1.2 NeL Driver";
	}
};

// --------------------------------------------------

}

#endif // RK_OPENGL_H
