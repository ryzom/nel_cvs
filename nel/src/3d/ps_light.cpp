/** \file ps_light.cpp
 *
 * $Id: ps_light.cpp,v 1.7 2004/03/19 16:31:27 lecroart Exp $
 */

/* Copyright, 2000, 2001, 2002, 2003 Nevrax Ltd.
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
#include "3d/ps_light.h"
#include "3d/point_light_model.h"
#include "3d/scene.h"
#include "3d/particle_system.h"
#include "3d/ps_util.h"
#include "nel/misc/matrix.h"
#include "nel/misc/vector.h"

namespace NL3D
{


//***************************************************************************************************************
CPSLight::CPSLight() : _Color(CRGBA::White),
					   _ColorScheme(NULL),
					   _AttenStart(0.1f),
					   _AttenStartScheme(NULL),
					   _AttenEnd(1.f),
					   _AttenEndScheme(NULL)
{	
}

//***************************************************************************************************************
CPSLight::~CPSLight()
{
	if (_Owner && _Owner->getOwner())
	{
		// check that all lights have been deleted
		for(uint k = 0; k < _Lights.getSize(); ++k)
		{
			if (_Lights[k]) _Owner->getOwner()->getScene()->deleteModel(_Lights[k]);			
		}
	}
	else
	{
		#ifdef NL_DEBUG
			// check that all lights have been deleted
			for(uint k = 0; k < _Lights.getSize(); ++k)
			{
				nlassert(_Lights[k] == NULL); // error there's	leak!
			}
		#endif
	}
	delete _ColorScheme;
	delete _AttenStartScheme;
	delete _AttenEndScheme;
}

//***************************************************************************************************************
void CPSLight::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	CPSLocatedBindable::serial(f);
	// version 1 : in version 0, scheme where not resized correctly; Fixed in this version
	// version 0 : color, start attenuation radius, end attenuation radius.
	sint ver = f.serialVersion(1);	
	// color
	bool hasColorScheme = _ColorScheme != NULL;
	f.serial(hasColorScheme);
	if (hasColorScheme)
	{
		f.serialPolyPtr(_ColorScheme);
	}
	else
	{
		f.serial(_Color);
	}
	// Atten start
	bool hasAttenStartScheme = _AttenStartScheme != NULL;
	f.serial(hasAttenStartScheme);
	if (hasAttenStartScheme)
	{
		f.serialPolyPtr(_AttenStartScheme);
	}
	else
	{
		f.serial(_AttenStart);
	}
	// Atten end
	bool hasAttenEndScheme = _AttenEndScheme != NULL;
	f.serial(hasAttenEndScheme);
	if (hasAttenEndScheme)
	{
		f.serialPolyPtr(_AttenEndScheme);
	}
	else
	{
		f.serial(_AttenEnd);
	}
	
	// save # of lights
	if (ver == 0)
	{	
		uint32 dummyNumLights; // from old buggy version
		f.serial(dummyNumLights);
	}
	if (f.isReading())
	{		
		if (_Owner)
		{		
			resize(_Owner->getMaxSize());		
			for(uint k = 0; k < _Owner->getSize(); ++k)
			{
				newElement(NULL, 0);
			}
		}
		else
		{
			resize(0);
		}
	}	
}

//***************************************************************************************************************
uint32 CPSLight::getType(void) const
{
	return PSLight; 
}

//***************************************************************************************************************
void CPSLight::step(TPSProcessPass pass,TAnimationTime ellapsedTime,TAnimationTime realEt)
{	
	if (pass != PSMotion) 
	{
		if (pass == PSToolRender)
		{
			show();
		}
		return;
	}
	nlassert(_Owner);
	nlassert(_Owner->getOwner());
	CScene *scene = _Owner->getOwner()->getScene();
	const uint32 BATCH_SIZE = 512;	
	uint32 numLeftLights = _Lights.getSize();
	// avoid ctor call for color array
	uint8		   colorArray[BATCH_SIZE * sizeof(NLMISC::CRGBA)];
	NLMISC::CRGBA *colors = (NLMISC::CRGBA *) colorArray;
	float		  attenStart[BATCH_SIZE];
	float		  attenEnd[BATCH_SIZE];
	CPSAttrib<CPointLightModel *>::iterator lightIt = _Lights.begin();
	const CMatrix *convMat = &(getLocalToWorldMatrix());
	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin();
	CRGBA globalColor = _Owner->getOwner()->getGlobalColor();
	while (numLeftLights)
	{
		uint32 toProcess = std::min(numLeftLights, BATCH_SIZE);
		// compute colors
		NLMISC::CRGBA *colPointer;
		uint   colStride;
		if (_ColorScheme)
		{
			colPointer = (CRGBA *) _ColorScheme->make(_Owner, _Lights.getSize() - numLeftLights, colors, sizeof(CRGBA), toProcess, true);
			colStride = 1;
		}
		else
		{
			colPointer = &_Color;
			colStride = 0;
		}
		// compute start attenuation
		float *attenStartPointer;
		uint   attenStartStride;
		if (_AttenStartScheme)
		{
			attenStartPointer = (float *) _AttenStartScheme->make(_Owner, _Lights.getSize() - numLeftLights, attenStart, sizeof(float), toProcess, true);
			attenStartStride = 1;
		}
		else
		{
			attenStartPointer = &_AttenStart;
			attenStartStride = 0;
		}
		// compute end attenuation
		float *attenEndPointer;
		uint   attenEndStride;
		if (_AttenEndScheme)
		{
			attenEndPointer = (float *) _AttenEndScheme->make(_Owner, _Lights.getSize() - numLeftLights, attenEnd, sizeof(float), toProcess, true);
			attenEndStride = 1;
		}
		else
		{
			attenEndPointer = &_AttenEnd;
			attenEndStride = 0;
		}
		numLeftLights -= toProcess;
		do 
		{
			if (!*lightIt)
			{
				// light not created, create it from scene
				if (scene)
				{
					*lightIt = NLMISC::safe_cast<CPointLightModel *>(scene->createModel(PointLightModelId));					
					if (*lightIt)
					{
						(*lightIt)->setTransformMode(CTransform::RotEuler);
					}
				}
			}
			if (*lightIt)
			{
				NLMISC::CVector pos = *convMat * *posIt;
				CPointLightModel *plm = *lightIt;
				if (pos != plm->getPos()) plm->setPos(pos);
				CRGBA newCol = *colPointer;
				newCol.modulateFromColor(newCol, globalColor);
				if (newCol != plm->PointLight.getDiffuse()) 
				{					
					plm->PointLight.setColor(newCol);
				}
				colPointer += colStride;
				if (*attenStartPointer != plm->PointLight.getAttenuationBegin()
				    || *attenEndPointer != plm->PointLight.getAttenuationEnd()
				   )
				{
					plm->PointLight.setupAttenuation(*attenStartPointer, *attenEndPointer);
				}
				attenStartPointer += attenStartStride;
				attenEndPointer += attenEndStride;
			}
			++ lightIt;
			++ posIt;
		} 
		while(--toProcess);		
	}		
}

//***************************************************************************************************************
void CPSLight::setColor(NLMISC::CRGBA color)
{
	delete _ColorScheme;
	_ColorScheme = NULL;
	_Color = color;
}

//***************************************************************************************************************
void CPSLight::setColorScheme(CPSAttribMaker<NLMISC::CRGBA> *scheme)
{
	delete _ColorScheme;
	_ColorScheme = scheme;
	if (_Owner)
	{	
		if (_ColorScheme && _ColorScheme->hasMemory()) _ColorScheme->resize(_Owner->getMaxSize(), _Owner->getSize());
	}
}

//***************************************************************************************************************
void CPSLight::setAttenStart(float radius)
{
	nlassert(radius > 0.f);
	delete _AttenStartScheme;
	_AttenStartScheme =	NULL;
	_AttenStart = radius;
}

//***************************************************************************************************************
void CPSLight::setAttenStartScheme(CPSAttribMaker<float> *scheme)
{
	delete _AttenStartScheme;
	_AttenStartScheme = scheme;
	if (_Owner)
	{	
		if (_AttenStartScheme && _AttenStartScheme->hasMemory()) _AttenStartScheme->resize(_Owner->getMaxSize(), _Owner->getSize());
	}
}

//***************************************************************************************************************
void CPSLight::setAttenEnd(float radius)
{
	delete _AttenEndScheme;
	_AttenEndScheme = NULL;
	_AttenEnd = radius;
}

//***************************************************************************************************************
void CPSLight::setAttenEndScheme(CPSAttribMaker<float> *scheme)
{
	delete _AttenEndScheme;
	_AttenEndScheme = scheme;
	if (_Owner)
	{	
		if (_AttenEndScheme && _AttenEndScheme->hasMemory()) _AttenEndScheme->resize(_Owner->getMaxSize(), _Owner->getSize());
	}
}

//***************************************************************************************************************
void CPSLight::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	if (_ColorScheme && _ColorScheme->hasMemory()) _ColorScheme->newElement(emitterLocated, emitterIndex);
	if (_AttenStartScheme && _AttenStartScheme->hasMemory()) _AttenStartScheme->newElement(emitterLocated, emitterIndex);
	if (_AttenEndScheme && _AttenEndScheme->hasMemory()) _AttenEndScheme->newElement(emitterLocated, emitterIndex);
	_Lights.insert(NULL); // instance is created during step()
}

//***************************************************************************************************************
void CPSLight::deleteElement(uint32 index)
{
	if (_ColorScheme && _ColorScheme->hasMemory()) _ColorScheme->deleteElement(index);
	if (_AttenStartScheme && _AttenStartScheme->hasMemory()) _AttenStartScheme->deleteElement(index);
	if (_AttenEndScheme && _AttenEndScheme->hasMemory()) _AttenEndScheme->deleteElement(index);	
	if (_Lights[index])
	{	
		nlassert(_Owner && _Owner->getScene());
		_Owner->getScene()->deleteModel(_Lights[index]);		
	}
	_Lights.remove(index);
}

//***************************************************************************************************************
void CPSLight::resize(uint32 size)
{
	nlassert(size < (1 << 16));		
	if (_ColorScheme && _ColorScheme->hasMemory()) _ColorScheme->resize(size, getOwner() ? getOwner()->getSize() : 0);
	if (_AttenStartScheme && _AttenStartScheme->hasMemory()) _AttenStartScheme->resize(size, getOwner() ? getOwner()->getSize() : 0);
	if (_AttenEndScheme && _AttenEndScheme->hasMemory()) _AttenEndScheme->resize(size, getOwner() ? getOwner()->getSize() : 0);
	_Lights.resize(size);	
}

//***************************************************************************************************************
void CPSLight::releaseAllRef()
{
	CPSLocatedBindable::releaseAllRef();	
	// delete all lights, because pointer to the scene is lost after detaching from a system.
	for(uint k = 0; k < _Lights.getSize(); ++k)
	{
		if (_Lights[k])
		{
			nlassert(_Owner && _Owner->getScene()); // if there's an instance there must be a scene from which it was created.
			_Owner->getScene()->deleteModel(_Lights[k]);
			_Lights[k] = NULL;
		}
	}
}

//***************************************************************************************************************
void CPSLight::show()
{	
	uint32 index;
	CPSLocated *loc;
	CPSLocatedBindable *lb;
	_Owner->getOwner()->getCurrentEditedElement(loc, index, lb);

	NLMISC::CMatrix xzMat;
	xzMat.setRot(CVector::I, CVector::K, CVector::Null);
	NLMISC::CMatrix xyMat;
	xyMat.setRot(CVector::I, CVector::J, CVector::Null);
	NLMISC::CMatrix yzMat;
	yzMat.setRot(CVector::J, CVector::K, CVector::Null);


	getDriver()->setupModelMatrix(NLMISC::CMatrix::Identity);
	const uint numSubdiv = 32;
	// for each element, see if it is the selected element, and if yes, display in red
	for (uint k = 0; k < _Lights.getSize(); ++k)
	{
		float radiusStart = _AttenStartScheme ? _AttenStartScheme->get(_Owner, k) : _AttenStart;
		float radiusEnd = _AttenEndScheme ? _AttenEndScheme->get(_Owner, k) : _AttenEnd;
		NLMISC::clamp(radiusStart, 0.f, radiusEnd);
		const NLMISC::CRGBA colStart = (((lb == NULL || this == lb) && loc == _Owner && index == k)  ? CRGBA::Blue : CRGBA(0, 0, 127));
		const NLMISC::CRGBA colEnd = (((lb == NULL || this == lb) && loc == _Owner && index == k)  ? CRGBA::Red : CRGBA(127, 0, 0));
		//
		CPSUtil::displayDisc(*getDriver(), radiusStart, getLocalToWorldMatrix() * _Owner->getPos()[k], xzMat, numSubdiv, colStart);
		CPSUtil::displayDisc(*getDriver(), radiusStart, getLocalToWorldMatrix() * _Owner->getPos()[k], xyMat, numSubdiv, colStart);
		CPSUtil::displayDisc(*getDriver(), radiusStart, getLocalToWorldMatrix() * _Owner->getPos()[k], yzMat, numSubdiv, colStart);
		//
		CPSUtil::displayDisc(*getDriver(), radiusEnd, getLocalToWorldMatrix() * _Owner->getPos()[k], xzMat, numSubdiv, colEnd);
		CPSUtil::displayDisc(*getDriver(), radiusEnd, getLocalToWorldMatrix() * _Owner->getPos()[k], xyMat, numSubdiv, colEnd);
		CPSUtil::displayDisc(*getDriver(), radiusEnd, getLocalToWorldMatrix() * _Owner->getPos()[k], yzMat, numSubdiv, colEnd);
		//
	}
}

} // namespace NL3D
