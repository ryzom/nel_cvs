/** \file driver_opengl.cpp
 * OpenGL driver implementation
 *
 * $Id: driver_opengl.cpp,v 1.1 2000/10/26 15:33:07 viau Exp $
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

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include <gl/gl.h>
#include "driver_opengl.h"

// --------------------------------------------------

namespace NL3D
{

// --------------------------------------------------

__declspec(dllexport) IDriver* NL3D_createIDriverInstance(void)
{
	return( new CDriverGL );
}


// --------------------------------------------------

bool CDriverGL::init(void)
{
	return(true);
}

// --------------------------------------------------

ModeList CDriverGL::enumModes()
{
	sint		n;
	DEVMODE		devmode;
	ModeList	ML;
	GfxMode		Mode;

	n=0;
	while( EnumDisplaySettings(NULL, n, &devmode) )
	{
		Mode.Windowed=false;
		Mode.Width=devmode.dmPelsWidth;
		Mode.Height=devmode.dmPelsHeight;	
		Mode.Depth=devmode.dmBitsPerPel;	
		ML.push_back(Mode);
		n++;	
	}
	return(ML);
}

// --------------------------------------------------

bool CDriverGL::setDisplay(void* wnd, const GfxMode& mode)
{
	uint8					Depth;
	int						pf;

#ifdef WIN32
	_hWnd=(HWND)wnd;
	_hDC=GetDC(_hWnd);
	Depth=GetDeviceCaps(_hDC,BITSPIXEL);
	// ---
	memset(&_pfd,0,sizeof(_pfd));
	_pfd.nSize        = sizeof(_pfd);
	_pfd.nVersion     = 1;
	_pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	_pfd.iPixelType   = PFD_TYPE_RGBA;
	_pfd.cColorBits   = (char)Depth;
	_pfd.cDepthBits   = 16;
	_pfd.iLayerType   = PFD_MAIN_PLANE;
	pf=ChoosePixelFormat(_hDC,&_pfd);
	if (!pf) 
	{
		return(false);
	} 
	if ( !SetPixelFormat(_hDC,pf,&_pfd) ) 
	{
		return(false);
	} 
	DescribePixelFormat(_hDC,pf,sizeof(PIXELFORMATDESCRIPTOR),&_pfd);
	ReleaseDC(_hWnd,_hDC);
	// ---
	_hDC=GetDC(_hWnd);
    _hRC=wglCreateContext(_hDC);
    wglMakeCurrent(_hDC,_hRC);
#endif
	glMatrixMode(GL_MODELVIEW);
	glViewport(0,0,mode.Width,mode.Height);
	glOrtho(0,mode.Width,mode.Height,0,-1.0f,1.0f);	
	glDisable(GL_ALPHA_TEST);
	glDisable(GL_AUTO_NORMAL);
	glDisable(GL_BLEND);
	glDisable(GL_COLOR_MATERIAL);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DITHER);
	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	glEnable(GL_LINE_SMOOTH);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);
	return(true);
}

// --------------------------------------------------

bool CDriverGL::clear2D(CRGBA& rgba)
{
	glClearColor((float)rgba.R/255.0f,(float)rgba.G/255.0f,(float)rgba.B/255.0f,(float)rgba.A/255.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	return(true);
}

// --------------------------------------------------

bool CDriverGL::clearZBuffer(float zval)
{
	glClearDepth(zval);
	glClear(GL_DEPTH_BUFFER_BIT);
	return(true);
}

// --------------------------------------------------

bool CDriverGL::setupTexture(CTexture& tex)
{
	if (tex.DrvInfos==NULL)
	{
// Hardcore C++ style :
//		tex.DrvInfos=new CTextureDrvInfosGL;
//		ITextureDrvInfos* iinfos=static_cast<ITextureDrvInfos*>(tex.DrvInfos);
//		CTextureDrvInfosGL* infos=static_cast<CTextureDrvInfosGL*>(iinfos);

// C++ standard/intelligent style :
		CTextureDrvInfosGL* infos = new CTextureDrvInfosGL;
		tex.DrvInfos=infos;

// C style :
//		tex.DrvInfos= static_cast<CTextureDrvInfosGL*>(tex.DrvInfos);
//		tex.DrvInfos=new CTextureDrvInfosGL;
//		CTextureDrvInfosGL* infos=(CTextureDrvInfosGL*)((ITextureDrvInfos*)(tex.DrvInfos));

		glGenTextures(1,&infos->ID);
		glBindTexture(GL_TEXTURE_2D,infos->ID);
		glPixelStorei(GL_UNPACK_ALIGNMENT,1);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexEnvf(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
		glTexImage2D(GL_TEXTURE_2D,0,4,tex.getWidth(),tex.getHeight(),0,GL_RGBA,GL_UNSIGNED_BYTE,tex.getDataPointer());
	}
	return(true);
}

// --------------------------------------------------

bool CDriverGL::_setupVertexBuffer(CVertexBuffer& VB)
{
	return(true);
}

bool CDriverGL::activeVertexBuffer(CVertexBuffer& VB)
{
	uint32	flags;

	if (VB.DrvInfos==NULL)
	{
		if ( !_setupVertexBuffer(VB) )
		{
			return(false);
		}
	}

	glEnable(GL_VERTEX_ARRAY);
	glVertexPointer(3,GL_FLOAT,VB.getVertexSize(),VB.getVertexCoordPointer());

	flags=VB.getFlags();
	if (flags & IDRV_VF_RGBA)
	{
		glEnableClientState(GL_COLOR_ARRAY);
		glColorPointer(4,GL_UNSIGNED_BYTE,VB.getVertexSize(),VB.getColorPointer());
	}
	if (flags & IDRV_VF_UV[0])
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2,GL_FLOAT,VB.getVertexSize(),VB.getTexCoordPointer(0,0));
	}
	return(true);
}

// --------------------------------------------------

/*

  Driver::setupMaterial
  -> crée le shader s'il n'existe pas
  les infos shaders sont stockées dans IDriver
  renderstates : abstrait
  1 dans IDriver (le courant)
  1 dans chaque shader
  setupMaterial -> balance les renderstates du shader dans IDriver (seulement ceux necessaires)

*/

static bool convBlend(TBlend blend, GLenum& glenum)
{
	switch(blend)
	{
	case one:
		glenum=GL_ONE;
		return(true);
	case zero:
		glenum=GL_ZERO;
		return(true);
	case srcalpha:
		glenum=GL_SRC_ALPHA;
		return(true);
	case invsrcalpha:
		glenum=GL_ONE_MINUS_SRC_ALPHA;
		return(true);
	default:
		break;
	}
	return(false);
}

bool CDriverGL::setupMaterial(CMaterial& mat)
{
	CShaderGL*	pShader;
	GLenum		glenum;

	if (!mat.pShader)
	{
		mat.pShader=new CShaderGL;
	}

//	RKASSERT( dynamic_cast<CShaderGL*>((IShader*)(mat.pShader)) );
	pShader=static_cast<CShaderGL*>((IShader*)(mat.pShader));

	convBlend( mat.getSrcBlend(),glenum );
	pShader->SrcBlend=glenum;
	convBlend( mat.getDstBlend(),glenum );
	pShader->DstBlend=glenum;

	return(true);
}

// --------------------------------------------------

bool CDriverGL::render(CPrimitiveBlock& PB, CMaterial& Mat)
{
	ITextureDrvInfos*	iinfos;
	CTextureDrvInfosGL*	infosgl;

	iinfos=Mat.pTex[0]->DrvInfos;
	infosgl=static_cast<CTextureDrvInfosGL*>(iinfos);
	glBindTexture(GL_TEXTURE_2D,infosgl->ID);
	glDrawElements(GL_TRIANGLES,3*PB.getNumTri(),GL_UNSIGNED_INT,PB.getTriPointer());
	return(true);
}

// --------------------------------------------------

bool CDriverGL::swapBuffers(void)
{
	if ( !SwapBuffers(_hDC) )	
	{
		return(false);
	}
	return(true);
}

// --------------------------------------------------

bool CDriverGL::release(void)
{
	wglMakeCurrent(NULL,NULL);
	ReleaseDC(_hWnd,_hDC);
	wglDeleteContext(_hRC);
	return(true);
}

// --------------------------------------------------

}
