/** \file lens_flare.cpp
 * 
 *
 * $Id: lens_flare.cpp,v 1.1 2001/07/17 13:57:48 lecroart Exp $
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

#include <nel/misc/types_nl.h>
#include <nel/misc/vector.h>
#include <nel/misc/matrix.h>

#include <nel/3d/u_material.h>
#include <nel/3d/u_camera.h>
#include <nel/3d/u_driver.h>
//#include <nel/3d/u_vertex_buffer.h>
//#include <nel/3d/u_primitive_block.h>

#include "camera.h"
#include "client.h"

using namespace NLMISC;
using namespace NL3D;
using namespace std;


/**
 * A lens-flare class
 * \author Stephane Coutelas
 * \author Nevrax France
 * \date 2000
 */
class CLensFlare
{
	/// sunDirection from world's origin
	NLMISC::CVector _SunDirection;

	float _AlphaCoef;

	/// if axis segment is longer than this value then no lens flare is displayed
	static const float _MaxLensFlareLenght;

	/// flare
	struct _CFlare
	{
		NL3D::UMaterial *Material;

		float Width;
		float Height;

		float Location;

		float Scale;
		
		_CFlare(NL3D::UTexture * texture, float width, float height, float location, float scale)
		{
/*			// pre-setting material
			Material.initUnlit ();
			Material.setTexture (0, texture);
			Material.setBlendFunc( NL3D::CMaterial::srcalpha, NL3D::CMaterial::one );
			Material.setBlend(true);

			// quad dimension
			Width = width;
			Height = height;

			// location on the lens-flare ray
			Location = location;

			// texture scale
			Scale = scale;
*/		}
	};

	/// flares due to light
	std::vector<_CFlare *> _Flares;

public:

	/// constructor
	CLensFlare(NLMISC::CVector sunDirection)
	{
		_SunDirection = sunDirection;
		_AlphaCoef = 1.0f;
	}

	/// add a flare to the flare list
	void addFlare(NL3D::UTexture * texture, float width, float height, float location = 1.f, float scale = 1.f);

	void setAlphaCoef(float coef)
	{
		_AlphaCoef = coef;
	}

	/// lens flare display function
	void show();
};


const float CLensFlare::_MaxLensFlareLenght = 0.4f;


/*********************************************************\
					addFlare()
\*********************************************************/
void CLensFlare::addFlare(UTexture * texture, float width, float height, float location, float scale)
{
	_Flares.push_back(new _CFlare(texture, width, height, location, scale));
}


/*********************************************************\
						show()
\*********************************************************/
void CLensFlare::show()
{
/*	CMatrix mtx;
	mtx.identity();

	nlassert(Driver!=NULL && Camera!=NULL);

	Driver->setupViewport (CViewport());
	Driver->setupViewMatrix (mtx);
	Driver->setupModelMatrix (mtx);
	Driver->setFrustum (0.f, 1.f, 0.f, 1.f, -1.f, 1.f, false);

	// Determining axis "screen center - light" vector
	CMatrix cameraMatrix = Camera->getMatrix();
	cameraMatrix.invert();
	CVector light = cameraMatrix * _SunDirection;
	light = Camera->getFrustum().project(light);
	
	CVector screenCenter(0.5f,0.5f,0);
	CVector axis = light - screenCenter;

	if(axis.norm()>_MaxLensFlareLenght)
	{
		return;
	}

	// rendering flares
	vector<_CFlare *>::iterator itflr;
	for(itflr = _Flares.begin(); itflr!=_Flares.end(); itflr++)
	{
		
		(*itflr)->Material.setColor(NLMISC::CRGBA(255,255,255,(uint8)(_AlphaCoef*255)));
			
		CVertexBuffer vb;
		vb.setVertexFormat (IDRV_VF_XYZ|IDRV_VF_UV[0]);
		vb.setNumVertices (4);
		
		float xCenterQuad = screenCenter.x + (*itflr)->Location * axis.x;
		float yCenterQuad = screenCenter.y + (*itflr)->Location * axis.y;
		
		float x,y;
		
		x = xCenterQuad - (*itflr)->Width * (*itflr)->Scale / 2.f;
		y = yCenterQuad - (*itflr)->Height * (*itflr)->Scale / 2.f;
		vb.setVertexCoord (0, CVector (x, 0, y));
		
		x = xCenterQuad + (*itflr)->Width * (*itflr)->Scale / 2.f;
		y = yCenterQuad - (*itflr)->Height * (*itflr)->Scale / 2.f;
		vb.setVertexCoord (1, CVector (x, 0, y));
		
		x = xCenterQuad + (*itflr)->Width * (*itflr)->Scale / 2.f;
		y = yCenterQuad + (*itflr)->Height * (*itflr)->Scale / 2.f;
		vb.setVertexCoord (2, CVector (x, 0, y));
		
		x = xCenterQuad - (*itflr)->Width * (*itflr)->Scale / 2.f;
		y = yCenterQuad + (*itflr)->Height * (*itflr)->Scale / 2.f;
		vb.setVertexCoord (3, CVector (x, 0, y));
		
		vb.setTexCoord (0, 0, 0.f, 1.f);
		vb.setTexCoord (1, 0, 1.f, 1.f);
		vb.setTexCoord (2, 0, 1.f, 0.f);
		vb.setTexCoord (3, 0, 0.f, 0.f);
		Driver->activeVertexBuffer(vb);

		CPrimitiveBlock pb;
		pb.setNumQuad (1);
		pb.setQuad (0, 0, 1, 2, 3);

		Driver->render(pb, (*itflr)->Material);
	}
*/	
}


CLensFlare	*LensFlare; 

void initLensFlare ()
{
/*	CVector sunVector = 100000*CVector(0.075f, -1.0f, 0.25f);
	LensFlare = new CLensFlare (Driver, Camera, sunVector);

	ITexture *flareTexture1 = new CTextureFile ("flare01.tga");
	ITexture *flareTexture3 = new CTextureFile ("flare03.tga");
	ITexture *flareTexture4 = new CTextureFile ("flare04.tga");
	ITexture *flareTexture5 = new CTextureFile ("flare05.tga");
	ITexture *flareTexture6 = new CTextureFile ("flare06.tga");
	ITexture *flareTexture7 = new CTextureFile ("flare07.tga");
	
	float w = 30/800.0f;
	float h = 30/600.0f;

	// shine
	LensFlare->addFlare( flareTexture3, w, h, 1.f, 16.f);

	LensFlare->addFlare( flareTexture1, w, h, 1.f, 6.f );
	LensFlare->addFlare( flareTexture6, w, h, 1.3f, 1.2f );
	LensFlare->addFlare( flareTexture7, w, h, 1.0f, 3.f );
	LensFlare->addFlare( flareTexture6, w, h, 0.5f, 4.f );
	LensFlare->addFlare( flareTexture5, w, h, 0.2f, 2.f );
	LensFlare->addFlare( flareTexture7, w, h, 0.0f, 0.8f );
	LensFlare->addFlare( flareTexture7, w, h, -0.25f, 2.f );
	LensFlare->addFlare( flareTexture1, w, h, -0.4f, 1.f );
	LensFlare->addFlare( flareTexture4, w, h, -1.0f, 12.f );
	LensFlare->addFlare( flareTexture5, w, h, -0.6f, 6.f );
*/}

void updateLensFlare ()
{
	return ;

	// vector to sun
	//==============
	CVector sunVector = CVector(0.075f, -1.0f, 0.25f);

	// look's vector
	//==============
	CVector v2 = CVector(0,0,1); //TODO mettre le vecteur de la cam LocalArea->User.bodyHeading();
	CVector v1 = v2 ^ CVector(0,0,1);
	CVector v3 = v1 ^ v2;
	CMatrix viewmatrix;
	viewmatrix.identity();
	viewmatrix.setRot( v1, v2, v3, true );
	viewmatrix.rotateX( 0/*TODO mettre le pitch de la cam LocalArea->User.ViewPitch*/ );
	CVector userLook = viewmatrix.getJ();

	// cosinus between the two previous vectors
	//=========================================
	float cosAngle = sunVector*userLook/sunVector.norm();	

	// alpha
	//======
	float alphaf;
	if(cosAngle<0) 
	{
		alphaf = 0;
	}
	else
	{
		alphaf = 255*(float)(pow(cosAngle,20));
	}
	
	// landscape's masking sun ?
	//==========================
	CMatrix camMatrix;
	camMatrix = Camera->getMatrix();
	camMatrix.setPos(CVector::Null);
	camMatrix.invert();
	CVector tmp = camMatrix * sunVector;
	tmp = Camera->getFrustum().project(tmp);
	uint32	w,h;
	Driver->getWindowSize(w,h);
	float sunRadius = 24;
	CRect rect((uint32)(tmp.x*w)-(uint32)sunRadius,(uint32)(tmp.y*h)-(uint32)sunRadius,2*(uint32)sunRadius,2*(uint32)sunRadius);
	vector<float> zbuff;
	Driver->getZBufferPart(zbuff, rect);
	float view = 0.f;
	float sum = 0;
	sint i;
	for(i=0; i<(sint)zbuff.size(); i++)
	{
		if(zbuff[i]>=0.99999f) sum ++;
	}
	view = sum/(sunRadius*2*sunRadius*2);

	// quad for dazzle 
	//================
	uint8 alpha = (uint8)(alphaf*view/2.0f);
	if(alpha!=0)
	{
		Driver->drawQuad(0,0,1,1,CRGBA(255,255,255,alpha));
	}

	// Display lens-flare
	LensFlare->setAlphaCoef( 1.f - (float)cos(alphaf*view*Pi/(2.f*255.f)) );
	LensFlare->show();
}

void releaseLensFlare ()
{
	delete LensFlare;
	LensFlare = NULL;
}

