/** \file ps_shockwave.cpp
 * Shockwaves particles.
 *
 * $Id: ps_shockwave.cpp,v 1.1 2002/02/15 17:03:29 vizerie Exp $
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


#include "3d/ps_macro.h"
#include "3d/ps_shockwave.h"
#include "3d/driver.h"
#include "3d/texture_grouped.h"
#include "3d/ps_iterator.h"
#include "3d/particle_system.h"


namespace NL3D 
{							   

///////////////////////////
// constant definition   //
///////////////////////////

static const uint shockWaveBufSize = 64; // number of shockwave to be processed at once

/////////////////////////////////
// CPSShockWave implementation //
/////////////////////////////////


/** Well, we could have put a method template in CPSShockWave, but some compilers
  * want the definition of the methods in the header, and some compilers
  * don't want friend with function template, so we use a static method template of a friend class instead,
  * which gives us the same result :)
  */
class CPSShockWaveHelper
{
public:
	template <class T>	
	static void drawShockWave(T posIt, CPSShockWave &s, uint size, uint32 srcStep)
	{
		PARTICLES_CHECK_MEM;
		nlassert(s._Owner);

		const uint32 vSize = s._Vb.getVertexSize();
		IDriver *driver = s.getDriver();

		s._Owner->incrementNbDrawnParticles(size); // for benchmark purpose	

		s.setupDriverModelMatrix();
		driver->activeVertexBuffer(s._Vb);	

		static CPlaneBasis planeBasis[shockWaveBufSize];
		float       sizes[shockWaveBufSize];
		float       angles[shockWaveBufSize];
		
		uint leftToDo  = size, toProcess;
		T endIt ;
		uint8 *currVertex;
		uint k ;

		const float angleStep = 256.f / s._NbSeg;
		float currAngle;

		CPlaneBasis *ptCurrBasis;
		uint32	ptCurrBasisIncrement = s._PlaneBasisScheme ? 1 : 0;

		float *ptCurrSize;
		uint32 ptCurrSizeIncrement = s._SizeScheme ? 1 : 0;

		float *ptCurrAngle;
		uint32 ptCurrAngleIncrement = s._Angle2DScheme ? 1 : 0;

		CVector radVect, innerVect;

		float radiusRatio;

		do
		{
			currVertex = (uint8 *) s._Vb.getVertexCoordPointer();
			toProcess = leftToDo > shockWaveBufSize ? shockWaveBufSize : leftToDo;
			endIt = posIt + toProcess;
			if (s._SizeScheme)
			{
				ptCurrSize  = (float *) (s._SizeScheme->make(s._Owner, size - leftToDo, (void *) sizes, sizeof(float), toProcess, true, srcStep));			
			}
			else
			{
				ptCurrSize = &s._ParticleSize;
			}

			if (s._PlaneBasisScheme)
			{
				ptCurrBasis  = (CPlaneBasis *) (s._PlaneBasisScheme->make(s._Owner, size - leftToDo, (void *) planeBasis, sizeof(CPlaneBasis), toProcess, true, srcStep));			
			}
			else
			{
				ptCurrBasis = &s._PlaneBasis;
			}

			if (s._Angle2DScheme)
			{
				ptCurrAngle  = (float *) (s._Angle2DScheme->make(s._Owner, size - leftToDo, (void *) angles, sizeof(float), toProcess, true, srcStep));			
			}
			else
			{
				ptCurrAngle = &s._Angle2D;
			}
			

			s.updateVbColNUVForRender(size - leftToDo, toProcess, srcStep);
			/*
			if (_ColorScheme)
			{
				_ColorScheme->makeN(_Owner, size - leftToDo, (uint8 *) s._Vb.getVertexCoordPointer() + s._Vb.getColorOff(), vSize, toProcess, (_NbSeg + 1) << 1, srcStep );
			}
			*/
			
			do
			{			
				currAngle = *ptCurrAngle;
				if (fabsf(*ptCurrSize) > 10E-6)
				{
					radiusRatio = (*ptCurrSize - s._RadiusCut) / *ptCurrSize;
				}
				else
				{
					radiusRatio = 0.f;
				}

				for (k = 0; k <= s._NbSeg; ++k)
				{
					radVect = *ptCurrSize * (CPSUtil::getCos((sint32) currAngle) * ptCurrBasis->X + CPSUtil::getSin((sint32) currAngle) * ptCurrBasis->Y);
					innerVect = radiusRatio * radVect;
					CHECK_VERTEX_BUFFER(s._Vb, currVertex);
					* (CVector *) currVertex = *posIt + radVect;
					currVertex += vSize;
					CHECK_VERTEX_BUFFER(s._Vb, currVertex);
					* (CVector *) currVertex = *posIt + innerVect;
					currVertex += vSize;
					currAngle += angleStep;				
				}
				
				++posIt;
				ptCurrBasis +=  ptCurrBasisIncrement;
				ptCurrSize  +=  ptCurrSizeIncrement;
				ptCurrAngle  +=  ptCurrAngleIncrement;
			}
			while (posIt != endIt);			

			s._Pb.setNumQuad(toProcess * s._NbSeg);
			driver->render(s._Pb, s._Mat);
			leftToDo -= toProcess;		
		}
		while (leftToDo);
		PARTICLES_CHECK_MEM;
	}
};

///=================================================================================
CPSShockWave::CPSShockWave(uint nbSeg, float radiusCut, CSmartPtr<ITexture> tex) 
		:  _NbSeg(nbSeg)
		   , _RadiusCut(radiusCut)
		   , _UFactor(1.f)

{
	nlassert(nbSeg > 2 && nbSeg <= 64);
	setTexture(tex);
	init();
	_Name = std::string("ShockWave");
}

///=================================================================================
uint32 CPSShockWave::getMaxNumFaces(void) const
{
	nlassert(_Owner);
	return (_Owner->getMaxSize() * _NbSeg) << 1 ;
}

///=================================================================================
bool CPSShockWave::hasTransparentFaces(void)
{
	return getBlendingMode() != CPSMaterial::alphaTest ;
}

///=================================================================================
bool CPSShockWave::hasOpaqueFaces(void)
{
	return !hasTransparentFaces();
}

///=================================================================================
void CPSShockWave::setNbSegs(uint nbSeg)
{
	nlassert(nbSeg > 2 && nbSeg <= 64);
	_NbSeg = nbSeg;
	if (_Owner)
	{
		resize(_Owner->getMaxSize());
		notifyOwnerMaxNumFacesChanged();
	}
}

///=================================================================================
void CPSShockWave::setRadiusCut(float radiusCut)
{
	_RadiusCut = radiusCut;	
	if (_Owner)
	{
		resize(_Owner->getMaxSize());
	}
}

///=================================================================================
void	CPSShockWave::setUFactor(float value)
{
	nlassert(_Owner); // must be attached to an owner before to call this method
	_UFactor = value;
	resize(_Owner->getSize()); // resize also recomputes the UVs..
}

///=================================================================================
void CPSShockWave::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{	
	sint ver  = f.serialVersion(2);
	CPSParticle::serial(f);
	CPSColoredParticle::serialColorScheme(f);
	CPSSizedParticle::serialSizeScheme(f);
	CPSTexturedParticle::serialTextureScheme(f);
	CPSRotated3DPlaneParticle::serialPlaneBasisScheme(f);
	CPSRotated2DParticle::serialAngle2DScheme(f);
	serialMaterial(f);
	f.serial(_NbSeg, _RadiusCut);
	if (ver > 1)
	{
		f.serial(_UFactor);
	}
	init();	
}

///=================================================================================
void CPSShockWave::draw(bool opaque)
{
	PARTICLES_CHECK_MEM;	
	if (!_Owner->getSize()) return;	

	uint32 step;
	uint   numToProcess;
	computeSrcStep(step, numToProcess);	
	if (!numToProcess) return;


	
	/// update the material if the global color of the system is variable
	CParticleSystem &ps = *(_Owner->getOwner());
	/// update the material if the global color of the system is variable		
	if (_ColorScheme != NULL && ps.getColorAttenuationScheme() != NULL)
	{		
		CPSMaterial::forceModulateConstantColor(true, ps.getGlobalColor());		
	}
	else
	{
		forceModulateConstantColor(false);
		if (!ps.getColorAttenuationScheme())
		{
			_Mat.setColor(_Color);
		}
		else
		{
			NLMISC::CRGBA col;
			col.modulateFromColor(ps.getGlobalColor(), _Color);
			_Mat.setColor(col);
		}
	}
	//////

	
	if (step == (1 << 16))
	{
		CPSShockWaveHelper::drawShockWave(_Owner->getPos().begin(),				   
										  *this,
										  numToProcess,
										  step
										 );
	}
	else
	{		
		CPSShockWaveHelper::drawShockWave(TIteratorVectStep1616(_Owner->getPos().begin(), 0, step),				   
										  *this,
										  numToProcess,
										  step
										 );
	}
	
	PARTICLES_CHECK_MEM;
}

///=================================================================================
bool CPSShockWave::completeBBox(NLMISC::CAABBox &box) const
{
	// TODO : implement this
	return false;
}

///=================================================================================
void CPSShockWave::init(void)
{
	_Mat.setLighting(false);	
	_Mat.setZFunc(CMaterial::less);
	_Mat.setDoubleSided(true);
	updateMatAndVbForColor();
	updateMatAndVbForTexture();
}

///=================================================================================
void CPSShockWave::updateVbColNUVForRender(uint32 startIndex, uint32 size, uint32 srcStep)
{
	nlassert(_Owner);
	if (!size) return;
	if (_ColorScheme)
	{
		// compute the colors, each color is replicated n times...
		_ColorScheme->makeN(_Owner, startIndex, _Vb.getColorPointer(), _Vb.getVertexSize(), size, (_NbSeg + 1) << 1, srcStep);
	}

	if (_TexGroup) // if it has a constant texture we are sure it has been setupped before...
	{	
		sint32 textureIndex[shockWaveBufSize];
		const uint32 stride = _Vb.getVertexSize(), stride2 = stride << 1;
		uint8 *currUV = (uint8 *) _Vb.getTexCoordPointer();				
		uint k;		

		uint32 currIndexIncr;
		const sint32 *currIndex;		

		if (_TextureIndexScheme)
		{
			currIndex  = (sint32 *) (_TextureIndexScheme->make(_Owner, startIndex, textureIndex, sizeof(sint32), size, true, srcStep));			
			currIndexIncr = 1;
		}
		else
		{	
			currIndex = &_TextureIndex;
			currIndexIncr = 0;
		}
		
		while (--size)
		{
			// for now, we don't make texture index wrapping
			const CTextureGrouped::TFourUV &uvGroup = _TexGroup->getUVQuad((uint32) *currIndex);

			k = _NbSeg + 1;

			for (k = 0; k <= _NbSeg; ++k)
			{
				
				*(CUV *) currUV = uvGroup.uv0 + CUV(k * _UFactor, 0);
				*(CUV *) (currUV + stride) = uvGroup.uv3 + CUV(k * _UFactor, 0);				
				// point the next quad
				currUV += stride2;
			}
			while (--k);

			currIndex += currIndexIncr;
		}		
	}	
}

///=================================================================================
void CPSShockWave::updateMatAndVbForColor(void)
{
	if (!_ColorScheme)
	{
		_Vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::TexCoord0Flag);		
		/// _Mat.setColor(_Color);
	}
	else
	{
		_Vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag | CVertexBuffer::TexCoord0Flag);
		/// _Mat.setColor(CRGBA::White);
	}

	if (_Owner)
	{
		resize(_Owner->getMaxSize());
	}	
}

///=================================================================================
void CPSShockWave::updateMatAndVbForTexture(void)
{
	_Mat.setTexture(0, _TexGroup ? (ITexture *) _TexGroup : (ITexture *) _Tex);	
}

///=================================================================================
void CPSShockWave::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	newColorElement(emitterLocated, emitterIndex);
	newTextureIndexElement(emitterLocated, emitterIndex);
	newSizeElement(emitterLocated, emitterIndex);
	newAngle2DElement(emitterLocated, emitterIndex);
}

///=================================================================================
void CPSShockWave::deleteElement(uint32 index)
{
	deleteColorElement(index);
	deleteTextureIndexElement(index);
	deleteSizeElement(index);
	deleteAngle2DElement(index);
}

///=================================================================================
void CPSShockWave::resize(uint32 aSize)
{
	nlassert(aSize < (1 << 16));
	resizeColor(aSize);
	resizeTextureIndex(aSize);
	resizeSize(aSize);
	resizeAngle2D(aSize);

	const uint32 size = aSize > shockWaveBufSize ? shockWaveBufSize : aSize;

	_Vb.setNumVertices((size * (_NbSeg + 1)) << 1 );		

	_Pb.reserveQuad(size * _NbSeg);
	for (uint32 k = 0; k < size; ++k)
	{
		for (uint32 l = 0; l < _NbSeg; ++l)
		{	
			const uint32 index = ((k * (_NbSeg + 1)) + l) << 1;						
			_Pb.setQuad(l + (k * _NbSeg) , index , index + 2, index + 3, index + 1);			
			_Vb.setTexCoord(index, 0, CUV(l * _UFactor, 0));
			_Vb.setTexCoord(index + 1, 0, CUV(l * _UFactor, 1));			
		}

		const uint32 index = ((k * (_NbSeg + 1)) + _NbSeg) << 1;

		_Vb.setTexCoord(index, 0, CUV(_NbSeg * _UFactor, 0));
		_Vb.setTexCoord(index + 1, 0, CUV(_NbSeg * _UFactor, 1));			

	}
}

} // NL3D
