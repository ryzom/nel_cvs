/** \file driver_user.cpp
 * <File description>
 *
 * $Id: driver_user.cpp,v 1.22 2002/08/23 12:26:32 besson Exp $
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
#include "nel/misc/hierarchical_timer.h"


using namespace NLMISC;

namespace NL3D 
{

H_AUTO_DECL( NL3D_UI_Driver )
H_AUTO_DECL( NL3D_Render_DriverClearBuffer )
H_AUTO_DECL( NL3D_Render_DriverSwapBuffer )
H_AUTO_DECL( NL3D_Render_DriverDrawELT )

#define	NL3D_HAUTO_UI_DRIVER				H_AUTO_USE( NL3D_UI_Driver )
#define	NL3D_HAUTO_CLEAR_DRIVER				H_AUTO_USE( NL3D_Render_DriverClearBuffer )
#define	NL3D_HAUTO_SWAP_DRIVER				H_AUTO_USE( NL3D_Render_DriverSwapBuffer )
#define	NL3D_HAUTO_DRAW_DRIVER				H_AUTO_USE( NL3D_Render_DriverDrawELT )


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
	_VBQuadsColUv.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag | CVertexBuffer::TexCoord0Flag);
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
	NL3D_HAUTO_UI_DRIVER;

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
	NL3D_HAUTO_UI_DRIVER;

	_Driver->disableHardwareVertexProgram();
}
void			CDriverUser::disableHardwareVertexArrayAGP()
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->disableHardwareVertexArrayAGP();
}
void			CDriverUser::disableHardwareTextureShader()
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->disableHardwareTextureShader();
}

// ***************************************************************************
void			CDriverUser::setDisplay(const CMode &mode) 
{
	NL3D_HAUTO_UI_DRIVER;

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
	NL3D_HAUTO_UI_DRIVER;

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
bool			CDriverUser::activate(void)
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->activate();
}

// ***************************************************************************
bool			CDriverUser::isActive()
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->isActive();
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
	NL3D_HAUTO_UI_DRIVER;

	_CurrentMatrixContext.Scissor= sc;
	setupMatrixContext();
}
// ***************************************************************************
CScissor		CDriverUser::getScissor()
{
	NL3D_HAUTO_UI_DRIVER;

	return _CurrentMatrixContext.Scissor;
}
// ***************************************************************************
void			CDriverUser::setViewport(const CViewport &vp)
{
	NL3D_HAUTO_UI_DRIVER;

	_CurrentMatrixContext.Viewport= vp;
	setupMatrixContext();
}
// ***************************************************************************
CViewport		CDriverUser::getViewport()
{
	NL3D_HAUTO_UI_DRIVER;

	return _CurrentMatrixContext.Viewport;
}
// ***************************************************************************
void			CDriverUser::setFrustum(const CFrustum &frust) 
{
	NL3D_HAUTO_UI_DRIVER;

	_CurrentMatrixContext.Frustum= frust;
	setupMatrixContext();
}
// ***************************************************************************
CFrustum		CDriverUser::getFrustum() 
{
	NL3D_HAUTO_UI_DRIVER;

	return _CurrentMatrixContext.Frustum;
}
// ***************************************************************************
void			CDriverUser::setViewMatrix(const CMatrix &mat) 
{
	NL3D_HAUTO_UI_DRIVER;

	_CurrentMatrixContext.ViewMatrix= mat;
	setupMatrixContext();
}
// ***************************************************************************
CMatrix			CDriverUser::getViewMatrix() 
{
	NL3D_HAUTO_UI_DRIVER;

	return _CurrentMatrixContext.ViewMatrix;
}
// ***************************************************************************
void			CDriverUser::setModelMatrix(const CMatrix &mat) 
{
	NL3D_HAUTO_UI_DRIVER;

	_CurrentMatrixContext.ModelMatrix= mat;
	setupMatrixContext();
}
// ***************************************************************************
CMatrix			CDriverUser::getModelMatrix() 
{
	NL3D_HAUTO_UI_DRIVER;

	return _CurrentMatrixContext.ModelMatrix;
}



// ***************************************************************************
void			CDriverUser::setMatrixMode2D(const CFrustum &frust) 
{
	NL3D_HAUTO_UI_DRIVER;

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
	NL3D_HAUTO_UI_DRIVER;

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
	NL3D_HAUTO_DRAW_DRIVER;

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
	NL3D_HAUTO_DRAW_DRIVER;

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
	NL3D_HAUTO_DRAW_DRIVER;

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
	NL3D_HAUTO_DRAW_DRIVER;

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
	NL3D_HAUTO_DRAW_DRIVER;

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
	NL3D_HAUTO_DRAW_DRIVER;

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
	NL3D_HAUTO_DRAW_DRIVER;

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
	NL3D_HAUTO_DRAW_DRIVER;

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
	NL3D_HAUTO_DRAW_DRIVER;

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
	NL3D_HAUTO_DRAW_DRIVER;

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
	NL3D_HAUTO_DRAW_DRIVER;

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
	NL3D_HAUTO_DRAW_DRIVER;

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
void			CDriverUser::drawQuads(const std::vector<NLMISC::CQuadColorUV> &q, UMaterial &mat)
{
	const CQuadColorUV *qptr = &(q[0]);
	drawQuads(qptr , q.size(), mat);
}

// ***************************************************************************
void			CDriverUser::drawQuads(const NLMISC::CQuadColorUV *quads, uint32 nbQuads, UMaterial &mat)
{
	NL3D_HAUTO_DRAW_DRIVER;

	CVertexBuffer		&vb = _VBQuadsColUv;

	vb.setNumVertices (4*nbQuads);

	for (uint32 i = 0; i < nbQuads; ++i)
	{
		const NLMISC::CQuadColorUV &qcuv = quads[i];
		vb.setVertexCoord (i*4+0, qcuv.V0);
		vb.setVertexCoord (i*4+1, qcuv.V1);
		vb.setVertexCoord (i*4+2, qcuv.V2);
		vb.setVertexCoord (i*4+3, qcuv.V3);
		vb.setColor(i*4+0, qcuv.Color0);
		vb.setColor(i*4+1, qcuv.Color1);
		vb.setColor(i*4+2, qcuv.Color2);
		vb.setColor(i*4+3, qcuv.Color3);
		vb.setTexCoord (i*4+0, 0, qcuv.Uv0);
		vb.setTexCoord (i*4+1, 0, qcuv.Uv1);
		vb.setTexCoord (i*4+2, 0, qcuv.Uv2);
		vb.setTexCoord (i*4+3, 0, qcuv.Uv3);
	}
	
	_Driver->activeVertexBuffer(vb);
	_Driver->renderQuads(convMat(mat), 0, nbQuads);
}


// ***************************************************************************
// ***************************************************************************
// 2D TOOLS.
// ***************************************************************************
// ***************************************************************************


// ***************************************************************************
void			CDriverUser::drawBitmap (float x, float y, float width, float height, class UTexture& texture, bool blend, CRGBA col) 
{
	NL3D_HAUTO_DRAW_DRIVER;

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
	NL3D_HAUTO_DRAW_DRIVER;

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
	NL3D_HAUTO_DRAW_DRIVER;

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
	NL3D_HAUTO_DRAW_DRIVER;

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


// ***************************************************************************
void			CDriverUser::clearRGBABuffer(CRGBA col)
{
	NL3D_HAUTO_CLEAR_DRIVER;

	_Driver->clear2D(col);
}
// ***************************************************************************
void			CDriverUser::clearZBuffer()
{
	NL3D_HAUTO_CLEAR_DRIVER;

	_Driver->clearZBuffer();
}
// ***************************************************************************
void			CDriverUser::clearBuffers(CRGBA col)
{
	NL3D_HAUTO_CLEAR_DRIVER;

	_Driver->clear2D(col);
	_Driver->clearZBuffer();
}
// ***************************************************************************
void			CDriverUser::swapBuffers()
{
	NL3D_HAUTO_SWAP_DRIVER;

	_Driver->swapBuffers();
}


// ***************************************************************************
bool			CDriverUser::fogEnabled()
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->fogEnabled();
}
// ***************************************************************************
void			CDriverUser::enableFog(bool enable)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->enableFog(enable);
}
// ***************************************************************************
void			CDriverUser::setupFog(float start, float end, CRGBA color)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->setupFog(start, end, color);
}


// ***************************************************************************
void			CDriverUser::setLight (uint8 num, const ULight& light)
{
	NL3D_HAUTO_UI_DRIVER;

	CLightUser	*plight= (CLightUser*)&light;
	_Driver->setLight (num, plight->_Light);
}
// ***************************************************************************
void			CDriverUser::enableLight (uint8 num, bool enable)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->enableLight (num, enable);
}
// ***************************************************************************
void			CDriverUser::setAmbientColor (CRGBA color)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->setAmbientColor (color);
}


// ***************************************************************************
void			CDriverUser::setPolygonMode (TPolygonMode mode)
{
	NL3D_HAUTO_UI_DRIVER;

	IDriver::TPolygonMode	dmode=IDriver::Filled;
	switch(mode)
	{
		case Filled:	dmode= IDriver::Filled; break;
		case Line:		dmode= IDriver::Line; break;
		case Point:		dmode= IDriver::Point; break;
		default: nlstop;
	};
	_Driver->setPolygonMode (dmode);
}
U3dMouseListener*	CDriverUser::create3dMouseListener ()
{
	NL3D_HAUTO_UI_DRIVER;

	// Alloc the listener
	CEvent3dMouseListener *listener=new CEvent3dMouseListener();

	// register it
	listener->addToServer (EventServer);

	return listener;
}
void CDriverUser::delete3dMouseListener (U3dMouseListener *listener)
{
	NL3D_HAUTO_UI_DRIVER;

	// Unregister
	((CEvent3dMouseListener*)listener)->removeFromServer (EventServer);

	delete (CEvent3dMouseListener*)listener;
}
UDriver::TPolygonMode 	CDriverUser::getPolygonMode () 
{
	NL3D_HAUTO_UI_DRIVER;

	IDriver::TPolygonMode	dmode;
	UDriver::TPolygonMode	umode=UDriver::Filled;
	dmode= _Driver->getPolygonMode();
	switch(dmode)
	{
		case IDriver::Filled:	umode= UDriver::Filled; break;
		case IDriver::Line:		umode= UDriver::Line; break;
		case IDriver::Point:	umode= UDriver::Point; break;
		default: nlstop;
	};

	return umode;
}
void			CDriverUser::forceDXTCCompression(bool dxtcComp)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->forceDXTCCompression(dxtcComp);
}
void			CDriverUser::forceTextureResize(uint divisor)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->forceTextureResize(divisor);
}



// ***************************************************************************
// ***************************************************************************
/// Driver information/Queries
// ***************************************************************************
// ***************************************************************************

uint32			CDriverUser::getImplementationVersion () const
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->getImplementationVersion ();
}
const char*		CDriverUser::getDriverInformation ()
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->getDriverInformation();
}
const char*		CDriverUser::getVideocardInformation ()
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->getVideocardInformation ();
}
sint			CDriverUser::getNbTextureStages()
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->getNbTextureStages();
}
void			CDriverUser::getWindowSize (uint32 &width, uint32 &height)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->getWindowSize (width, height);
}
void			CDriverUser::getBuffer (CBitmap &bitmap) 
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->getBuffer (bitmap) ;
}
void			CDriverUser::getZBuffer (std::vector<float>  &zbuffer) 
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->getZBuffer (zbuffer) ;
}
void			CDriverUser::getBufferPart (CBitmap &bitmap, NLMISC::CRect &rect) 
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->getBufferPart (bitmap, rect) ;
}
void			CDriverUser::getZBufferPart (std::vector<float>  &zbuffer, NLMISC::CRect &rect) 
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->getZBufferPart (zbuffer, rect) ;
}
bool			CDriverUser::fillBuffer (CBitmap &bitmap)
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->fillBuffer(bitmap);
}


// ***************************************************************************
// ***************************************************************************
// Mouse / Keyboards / Game devices
// ***************************************************************************
// ***************************************************************************

NLMISC::IMouseDevice			*CDriverUser::enableLowLevelMouse(bool enable)
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->enableLowLevelMouse(enable);
}
NLMISC::IKeyboardDevice			*CDriverUser::enableLowLevelKeyboard(bool enable) 
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->enableLowLevelKeyboard(enable);
}
NLMISC::IInputDeviceManager		*CDriverUser::getLowLevelInputDeviceManager()
{
	NL3D_HAUTO_UI_DRIVER;

	return _Driver->getLowLevelInputDeviceManager();
}
void			CDriverUser::showCursor (bool b)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->showCursor(b);
}
void			CDriverUser::setMousePos (float x, float y)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->setMousePos (x, y);
}
void			CDriverUser::setCapture (bool b)
{
	NL3D_HAUTO_UI_DRIVER;

	_Driver->setCapture (b);
}

} // NL3D
