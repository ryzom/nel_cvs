#ifndef RK_OPENGL_H
#define RK_OPENGL_H

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <gl/gl.h>
#include "nel/3d/driver.h"
#include "nel/misc/types_nl.h"
#include "nel/misc/smart_ptr.h"
#include "nel/misc/rgba.h"

namespace NL3D
{

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
#ifdef WIN32
	HWND					_hWnd;
	HDC						_hDC;
	PIXELFORMATDESCRIPTOR	_pfd;
    HGLRC					_hRC;
#endif
	CShaderGL				_Shader;

	bool					_setupVertexBuffer(CVertexBuffer& VB);

public:
							CDriverGL() {};
	virtual					~CDriverGL() { release(); };

	virtual bool			init(void);

	virtual ModeList		enumModes();

	virtual bool			setDisplay(void* wnd, const GfxMode& mode);

	virtual bool			clear2D(CRGBA& rgba);

	virtual bool			clearZBuffer(float zval=0);

	virtual bool			setupTexture(CTexture& tex);

	virtual bool			setupMaterial(CMaterial& mat);

	virtual bool			activeVertexBuffer(CVertexBuffer& VB);

	virtual bool			render(CPrimitiveBlock& PB, CMaterial& Mat);

	virtual bool			swapBuffers(void);

	virtual bool			release(void);
};

  
// --------------------------------------------------

}

#endif // RK_OPENGL_H
