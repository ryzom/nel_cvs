/** \file driver_user.cpp
 * <File description>
 *
 * $Id: driver_user.cpp,v 1.17 2002/02/28 12:59:49 besson Exp $
 */

/* Copyright, 2001 Nevrax Ltd.
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

#include "std3d.h"

// ***************************************************************************
// THIS FILE IS DIVIDED IN TWO PARTS BECAUSE IT MAKES VISUAL CRASH.
// fatal error C1076: compiler limit : internal heap limit reached; use /Zm to specify a higher limit
// ***************************************************************************

#include "3d/driver_user.h"
#include "nel/3d/u_driver.h"
#include "3d/dru.h"
#include "3d/scene.h"
#include "3d/text_context_user.h"
#include "3d/texture_user.h"
#include "3d/material_user.h"
#include "3d/scene_user.h"
#include "3d/init_3d.h"
#include "nel/3d/u_camera.h"


using namespace NLMISC;

namespace NL3D 
{




// ***************************************************************************
// ***************************************************************************
// UDriver implementation (gcc link bug force to not put them in .h).
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
UDriver::UDriver()
{
}
// ***************************************************************************
UDriver::~UDriver()
{
}


// ***************************************************************************
void					UDriver::setMatrixMode2D11()
{
	setMatrixMode2D(CFrustum(0.0f,1.0f,0.0f,1.0f,-1.0f,1.0f,false));
}
// ***************************************************************************
void					UDriver::setMatrixMode2D43()
{
	setMatrixMode2D(CFrustum(0.0f,4.0f/3.0f,0.0f,1.0f,-1.0f,1.0f,false));
}


// ***************************************************************************
UDriver					*UDriver::createDriver()
{
	return new CDriverUser;
}


// ***************************************************************************
// ***************************************************************************
// WINDOWS Management.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
bool	CDriverUser::_StaticInit= false;


// ***************************************************************************
CDriverUser::CDriverUser()
{
	// The enum of IDriver and UDriver MUST be the same!!!
	nlassert((uint)IDriver::idCount == (uint)UDriver::idCount);
	nlassert((uint)IDriver::typeCount == (uint)UDriver::typeCount);
	nlassert((uint)IDriver::iconCount == (uint)UDriver::iconCount);


	// Static Initialisation.
	if(!_StaticInit)
	{
		_StaticInit= true;
		// Register basic serial.
		NL3D::registerSerial3d();

		// Register basic csene.
		CScene::registerBasics();
	}

	// Create/Init Driver.
	_Driver= CDRU::createGlDriver();
	nlassert(_Driver);
	_Driver->init();

	_WindowInit= false;

	// Init of VBuffers/PBs for 2D/3D interface.
	_VBFlat.setVertexFormat(CVertexBuffer::PositionFlag);
	_VBColor.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag);
	_VBUv.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag);
	_VBColorUv.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag | CVertexBuffer::TexCoord0Flag);
	// max is quad.
	_VBFlat.setNumVertices(4);
	_VBColor.setNumVertices(4);
	_VBUv.setNumVertices(4);
	_VBColorUv.setNumVertices(4); 

	_PBLine.setNumLine(1);
	_PBLine.setLine(0, 0, 1);
	_PBTri.setNumTri(1);
	_PBTri.setTri(0, 0, 1, 2);
	_PBQuad.setNumQuad(1);
	_PBQuad.setQuad(0, 0, 1, 2, 3);

}
// ***************************************************************************
CDriverUser::~CDriverUser()
{
	release();

	delete _Driver;
	_Driver= NULL;
}

// ***************************************************************************
UDriver::TModeList		CDriverUser::enumModes()
{
	ModeList	dlist;
	TModeList	retlist;

	dlist= _Driver->enumModes();
	for(sint i=0;i<(sint)dlist.size();i++)
	{
		retlist.push_back(CMode(dlist[i].Width, dlist[i].Height, dlist[i].Depth, dlist[i].Windowed));
	}

	return retlist;
}

// ***************************************************************************
void			CDriverUser::disableHardwareVertexProgram()
{
	_Driver->disableHardwareVertexProgram();
}
void			CDriverUser::disableHardwareVertexArrayAGP()
{
	_Driver->disableHardwareVertexArrayAGP();
}
void			CDriverUser::disableHardwareTextureShader()
{
	_Driver->disableHardwareTextureShader();
}

// ***************************************************************************
void			CDriverUser::setDisplay(const CMode &mode) 
{
	// window init.
	nlverify(_Driver->setDisplay(NULL, GfxMode(mode.Width, mode.Height, mode.Depth, mode.Windowed)));
	nlverify(activate());
	_WindowInit= true;

	// Event init.
	AsyncListener.reset ();
	EventServer.addEmitter(_Driver->getEventEmitter());
	AsyncListener.addToServer(EventServer);

	// Matrix Context (2D).
	_CurrentMatrixContext.Viewport.initFullScreen();
	_CurrentMatrixContext.Scissor.initFullScreen();
	setMatrixMode2D11();

	// 2D Material.
	_MatFlat.initUnlit();
	_MatFlat.setZFunc(UMaterial::always);
	_MatFlat.setZWrite(false);
	_MatText.initUnlit();
	_MatText.setZFunc(UMaterial::always);
	_MatText.setZWrite(false);

}
// ***************************************************************************
void			CDriverUser::release() 
{
	if(!_WindowInit)
		return;

	// 2D Material.
	_MatFlat.initUnlit();
	_MatText.initUnlit();

	// delete Texture, mat ... list.
	_Textures.clear();
	_Materials.clear();
	_TextContexts.clear();
	_Scenes.clear();

	// release event.
	AsyncListener.removeFromServer(EventServer);
	EventServer.removeEmitter(_Driver->getEventEmitter());

	// release window.
	_Driver->release();
	_WindowInit= false;
}



// ***************************************************************************
// ***************************************************************************
// Matrix context.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CDriverUser::setupMatrixContext()
{
	_Driver->setupScissor(_CurrentMatrixContext.Scissor);
	_Driver->setupViewport(_CurrentMatrixContext.Viewport);
	CFrustum	&f= _CurrentMatrixContext.Frustum;
	_Driver->setFrustum(f.Left, f.Right, f.Bottom, f.Top, f.Near, f.Far, f.Perspective);
	_Driver->setupViewMatrix(_CurrentMatrixContext.ViewMatrix);
	_Driver->setupModelMatrix(_CurrentMatrixContext.ModelMatrix);
}
// ***************************************************************************
void			CDriverUser::setScissor(const CScissor &sc)
{
	_CurrentMatrixContext.Scissor= sc;
	setupMatrixContext();
}
// ***************************************************************************
CScissor		CDriverUser::getScissor()
{
	return _CurrentMatrixContext.Scissor;
}
// ***************************************************************************
void			CDriverUser::setViewport(const CViewport &vp)
{
	_CurrentMatrixContext.Viewport= vp;
	setupMatrixContext();
}
// ***************************************************************************
CViewport		CDriverUser::getViewport()
{
	return _CurrentMatrixContext.Viewport;
}
// ***************************************************************************
void			CDriverUser::setFrustum(const CFrustum &frust) 
{
	_CurrentMatrixContext.Frustum= frust;
	setupMatrixContext();
}
// ***************************************************************************
CFrustum		CDriverUser::getFrustum() 
{
	return _CurrentMatrixContext.Frustum;
}
// ***************************************************************************
void			CDriverUser::setViewMatrix(const CMatrix &mat) 
{
	_CurrentMatrixContext.ViewMatrix= mat;
	setupMatrixContext();
}
// ***************************************************************************
CMatrix			CDriverUser::getViewMatrix() 
{
	return _CurrentMatrixContext.ViewMatrix;
}
// ***************************************************************************
void			CDriverUser::setModelMatrix(const CMatrix &mat) 
{
	_CurrentMatrixContext.ModelMatrix= mat;
	setupMatrixContext();
}
// ***************************************************************************
CMatrix			CDriverUser::getModelMatrix() 
{
	return _CurrentMatrixContext.ModelMatrix;
}



// ***************************************************************************
void			CDriverUser::setMatrixMode2D(const CFrustum &frust) 
{
	_CurrentMatrixContext.Frustum= frust;
	// We still work in NL3D coordinates, so must convert y to z.
	CVector		I(1,0,0);
	CVector		J(0,0,1);
	CVector		K(0,-1,0);

	_CurrentMatrixContext.ViewMatrix.identity();
	_CurrentMatrixContext.ViewMatrix.setRot(I,J,K, true);
	_CurrentMatrixContext.ModelMatrix.identity();

	setupMatrixContext();
}
// ***************************************************************************
void			CDriverUser::setMatrixMode3D(UCamera &camera) 
{
	// Retrieve the matrix and frustum from the camera.
	CMatrix		viewMat;
	viewMat= camera.getMatrix();
	viewMat.invert();
	setViewMatrix(viewMat);
	setModelMatrix(CMatrix::Identity);
	setFrustum(camera.getFrustum());
}


// ***************************************************************************
// ***************************************************************************
// 2D/3D INTERFACE.
// ***************************************************************************
// ***************************************************************************

// ***************************************************************************
void			CDriverUser::drawLine(const NLMISC::CLine &shp, UMaterial &mat) 
{
	CVertexBuffer		&vb= _VBFlat;
	CPrimitiveBlock		&pb= _PBLine;

	vb.setVertexCoord (0, shp.V0);
	vb.setVertexCoord (1, shp.V1);
	
	_Driver->activeVertexBuffer(vb);
	_Driver->render(pb, convMat(mat));
}
// ***************************************************************************
void			CDriverUser::drawLine(const NLMISC::CLineColor &shp, UMaterial &mat) 
{
	CVertexBuffer		&vb= _VBColor;
	CPrimitiveBlock		&pb= _PBLine;

	vb.setVertexCoord (0, shp.V0);
	vb.setVertexCoord (1, shp.V1);
	vb.setColor(0, shp.Color0);
	vb.setColor(1, shp.Color1);
	
	_Driver->activeVertexBuffer(vb);
	_Driver->render(pb, convMat(mat));
}
// ***************************************************************************
void			CDriverUser::drawLine(const NLMISC::CLineUV &shp, UMaterial &mat) 
{
	CVertexBuffer		&vb= _VBUv;
	CPrimitiveBlock		&pb= _PBLine;

	vb.setVertexCoord (0, shp.V0);
	vb.setVertexCoord (1, shp.V1);
	vb.setTexCoord (0, 0, shp.Uv0);
	vb.setTexCoord (1, 0, shp.Uv1);
	
	_Driver->activeVertexBuffer(vb);
	_Driver->render(pb, convMat(mat));
}
// ***************************************************************************
void			CDriverUser::drawLine(const NLMISC::CLineColorUV &shp, UMaterial &mat) 
{
	CVertexBuffer		&vb= _VBColorUv;
	CPrimitiveBlock		&pb= _PBLine;

	vb.setVertexCoord (0, shp.V0);
	vb.setVertexCoord (1, shp.V1);
	vb.setColor(0, shp.Color0);
	vb.setColor(1, shp.Color1);
	vb.setTexCoord (0, 0, shp.Uv0);
	vb.setTexCoord (1, 0, shp.Uv1);
	
	_Driver->activeVertexBuffer(vb);
	_Driver->render(pb, convMat(mat));
}



// ***************************************************************************
void			CDriverUser::drawTriangle(const NLMISC::CTriangle &shp, UMaterial &mat) 
{
	CVertexBuffer		&vb= _VBFlat;
	CPrimitiveBlock		&pb= _PBTri;

	vb.setVertexCoord (0, shp.V0);
	vb.setVertexCoord (1, shp.V1);
	vb.setVertexCoord (2, shp.V2);
	
	_Driver->activeVertexBuffer(vb);
	_Driver->render(pb, convMat(mat));
}
// ***************************************************************************
void			CDriverUser::drawTriangle(const NLMISC::CTriangleColor &shp, UMaterial &mat) 
{
	CVertexBuffer		&vb= _VBColor;
	CPrimitiveBlock		&pb= _PBTri;

	vb.setVertexCoord (0, shp.V0);
	vb.setVertexCoord (1, shp.V1);
	vb.setVertexCoord (2, shp.V2);
	vb.setColor(0, shp.Color0);
	vb.setColor(1, shp.Color1);
	vb.setColor(2, shp.Color2);
	
	_Driver->activeVertexBuffer(vb);
	_Driver->render(pb, convMat(mat));
}
// ***************************************************************************
void			CDriverUser::drawTriangle(const NLMISC::CTriangleUV &shp, UMaterial &mat) 
{
	CVertexBuffer		&vb= _VBUv;
	CPrimitiveBlock		&pb= _PBTri;

	vb.setVertexCoord (0, shp.V0);
	vb.setVertexCoord (1, shp.V1);
	vb.setVertexCoord (2, shp.V2);
	vb.setTexCoord (0, 0, shp.Uv0);
	vb.setTexCoord (1, 0, shp.Uv1);
	vb.setTexCoord (2, 0, shp.Uv2);
	
	_Driver->activeVertexBuffer(vb);
	_Driver->render(pb, convMat(mat));
}
// ***************************************************************************
void			CDriverUser::drawTriangle(const NLMISC::CTriangleColorUV &shp, UMaterial &mat) 
{
	CVertexBuffer		&vb= _VBColorUv;
	CPrimitiveBlock		&pb= _PBTri;

	vb.setVertexCoord (0, shp.V0);
	vb.setVertexCoord (1, shp.V1);
	vb.setVertexCoord (2, shp.V2);
	vb.setColor(0, shp.Color0);
	vb.setColor(1, shp.Color1);
	vb.setColor(2, shp.Color2);
	vb.setTexCoord (0, 0, shp.Uv0);
	vb.setTexCoord (1, 0, shp.Uv1);
	vb.setTexCoord (2, 0, shp.Uv2);
	
	_Driver->activeVertexBuffer(vb);
	_Driver->render(pb, convMat(mat));
}



// ***************************************************************************
void			CDriverUser::drawQuad(const NLMISC::CQuad &shp, UMaterial &mat) 
{
	CVertexBuffer		&vb= _VBFlat;
	CPrimitiveBlock		&pb= _PBQuad;

	vb.setVertexCoord (0, shp.V0);
	vb.setVertexCoord (1, shp.V1);
	vb.setVertexCoord (2, shp.V2);
	vb.setVertexCoord (3, shp.V3);
	
	_Driver->activeVertexBuffer(vb);
	_Driver->render(pb, convMat(mat));
}
// ***************************************************************************
void			CDriverUser::drawQuad(const NLMISC::CQuadColor &shp, UMaterial &mat) 
{
	CVertexBuffer		&vb= _VBColor;
	CPrimitiveBlock		&pb= _PBQuad;

	vb.setVertexCoord (0, shp.V0);
	vb.setVertexCoord (1, shp.V1);
	vb.setVertexCoord (2, shp.V2);
	vb.setVertexCoord (3, shp.V3);
	vb.setColor(0, shp.Color0);
	vb.setColor(1, shp.Color1);
	vb.setColor(2, shp.Color2);
	vb.setColor(3, shp.Color3);
	
	_Driver->activeVertexBuffer(vb);
	_Driver->render(pb, convMat(mat));
}
// ***************************************************************************
void			CDriverUser::drawQuad(const NLMISC::CQuadUV &shp, UMaterial &mat) 
{
	CVertexBuffer		&vb= _VBUv;
	CPrimitiveBlock		&pb= _PBQuad;

	vb.setVertexCoord (0, shp.V0);
	vb.setVertexCoord (1, shp.V1);
	vb.setVertexCoord (2, shp.V2);
	vb.setVertexCoord (3, shp.V3);
	vb.setTexCoord (0, 0, shp.Uv0);
	vb.setTexCoord (1, 0, shp.Uv1);
	vb.setTexCoord (2, 0, shp.Uv2);
	vb.setTexCoord (3, 0, shp.Uv3);
	
	_Driver->activeVertexBuffer(vb);
	_Driver->render(pb, convMat(mat));
}
// ***************************************************************************
void			CDriverUser::drawQuad(const NLMISC::CQuadColorUV &shp, UMaterial &mat) 
{
	CVertexBuffer		&vb= _VBColorUv;
	CPrimitiveBlock		&pb= _PBQuad;

	vb.setVertexCoord (0, shp.V0);
	vb.setVertexCoord (1, shp.V1);
	vb.setVertexCoord (2, shp.V2);
	vb.setVertexCoord (3, shp.V3);
	vb.setColor(0, shp.Color0);
	vb.setColor(1, shp.Color1);
	vb.setColor(2, shp.Color2);
	vb.setColor(3, shp.Color3);
	vb.setTexCoord (0, 0, shp.Uv0);
	vb.setTexCoord (1, 0, shp.Uv1);
	vb.setTexCoord (2, 0, shp.Uv2);
	vb.setTexCoord (3, 0, shp.Uv3);
	
	_Driver->activeVertexBuffer(vb);
	_Driver->render(pb, convMat(mat));
}


// ***************************************************************************
// ***************************************************************************
// 2D TOOLS.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CDriverUser::drawBitmap (float x, float y, float width, float height, class UTexture& texture, bool blend, CRGBA col) 
{
	_MatText.setTexture(&texture);
	_MatText.setColor(col);
	_MatText.setBlend(blend);

	CQuadUV		quad;
	quad.V0.set(x,y,0);
	quad.V1.set(x+width,y,0);
	quad.V2.set(x+width,y+height,0);
	quad.V3.set(x,y+height,0);
	quad.Uv0.U= 0.f;
	quad.Uv0.V= 1.f;
	quad.Uv1.U= 1.f;
	quad.Uv1.V= 1.f;
	quad.Uv2.U= 1.f;
	quad.Uv2.V= 0.f;
	quad.Uv3.U= 0.f;
	quad.Uv3.V= 0.f;

	drawQuad(quad, _MatText);
}
// ***************************************************************************
void			CDriverUser::drawLine (float x0, float y0, float x1, float y1, CRGBA col) 
{
	_MatFlat.setColor(col);
	_MatFlat.setBlend(true);

	CLine		line;
	line.V0.set(x0,y0,0);
	line.V1.set(x1,y1,0);

	drawLine(line, _MatFlat);
}
// ***************************************************************************
void			CDriverUser::drawTriangle (float x0, float y0, float x1, float y1, float x2, float y2, CRGBA col) 
{
	_MatFlat.setColor(col);
	_MatFlat.setBlend(true);

	CTriangle		tri;
	tri.V0.set(x0,y0,0);
	tri.V1.set(x1,y1,0);
	tri.V2.set(x2,y2,0);

	drawTriangle(tri, _MatFlat);
}
// ***************************************************************************
void			CDriverUser::drawQuad (float x0, float y0, float x1, float y1, CRGBA col) 
{
	_MatFlat.setColor(col);
	_MatFlat.setBlend(true);

	CQuad		quad;
	quad.V0.set(x0,y0,0);
	quad.V1.set(x1,y0,0);
	quad.V2.set(x1,y1,0);
	quad.V3.set(x0,y1,0);

	drawQuad(quad, _MatFlat);
}
// ***************************************************************************
void			CDriverUser::drawQuad (float xcenter, float ycenter, float radius, CRGBA col) 
{
	drawQuad(xcenter-radius, ycenter-radius, xcenter+radius, ycenter+radius, col);
}
// ***************************************************************************
void			CDriverUser::drawWiredQuad (float x0, float y0, float x1, float y1, CRGBA col) 
{
	// v-left
	drawLine(x0,y0,x0,y1,col);	
	// v-right
	drawLine(x1,y0,x1,y1,col);	
	// h-up
	drawLine(x0,y1,x1,y1,col);	
	// h-bottom
	drawLine(x0,y0,x1,y0,col);	
}
// ***************************************************************************
void			CDriverUser::drawWiredQuad (float xcenter, float ycenter, float radius, CRGBA col) 
{
	drawWiredQuad(xcenter-radius, ycenter-radius, xcenter+radius, ycenter+radius, col);
}


// ***************************************************************************
// ***************************************************************************
// MISC.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
UDriver::TMessageBoxId	CDriverUser::systemMessageBox (const char* message, const char* title, TMessageBoxType type, TMessageBoxIcon icon)
{
	IDriver::TMessageBoxId		dret;
	IDriver::TMessageBoxType	dtype= (IDriver::TMessageBoxType)(uint32)type;
	IDriver::TMessageBoxIcon	dicon= (IDriver::TMessageBoxIcon)(uint32)icon;
	dret= _Driver->systemMessageBox (message, title, dtype, dicon);

	return (UDriver::TMessageBoxId)(uint32)dret;
}


// ***************************************************************************
CMaterial		&CDriverUser::convMat(UMaterial &mat)
{
	CMaterialUser	*pmat= (CMaterialUser*)&mat;
	return pmat->_Material;
}

} // NL3D
