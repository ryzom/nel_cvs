/** \file ps_tail_dot.cpp
 * Tail dot particles.
 *
 * $Id: ps_tail_dot.cpp,v 1.1 2002/02/15 17:03:29 vizerie Exp $
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

#include "3d/ps_tail_dot.h"
#include "3d/ps_macro.h"
#include "3d/driver.h"
#include "3d/particle_system.h"

namespace NL3D 
{


///////////////////////////////
// CPSTailDot implementation //
///////////////////////////////

///==========================================================================================
// ctor
CPSTailDot::CPSTailDot(uint32 nbSegmentInTail) : _TailNbSeg(nbSegmentInTail), _ColorFading(true)
												 ,_SystemBasisEnabled(false)
{
	nlassert(_TailNbSeg <= 255);
	init();
	_Name = std::string("TailDot");
}

///==========================================================================================
uint32 CPSTailDot::getMaxNumFaces(void) const
{
	nlassert(_Owner);
	return _Owner->getMaxSize() * _TailNbSeg;
}

///==========================================================================================
bool CPSTailDot::hasTransparentFaces(void)
{
	return getBlendingMode() != CPSMaterial::alphaTest ;
}

///==========================================================================================
bool CPSTailDot::hasOpaqueFaces(void)
{
	return !hasTransparentFaces();
}


///==========================================================================================
void CPSTailDot::init(void)
{
	_Mat.setLighting(false);	
	_Mat.setZFunc(CMaterial::less);
	_Mat.setDoubleSided(true);

	_Vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag );	
}
	
///==========================================================================================
void CPSTailDot::step(TPSProcessPass pass, TAnimationTime ellapsedTime)
{
	if (
			(pass == PSBlendRender && hasTransparentFaces())
			|| (pass == PSSolidRender && hasOpaqueFaces())
			)
	{
		draw(pass == PSSolidRender);
	}
	else 
	if (pass == PSToolRender) // edition mode only
	{			
		showTool();
	}
	else if (pass == PSMotion)
	{
		PARTICLES_CHECK_MEM;
		// the number of particles
		const uint32 size = _Owner->getSize();

		if (!size) return;	

		// size of the vertices
		const uint32 vSize = _Vb.getVertexSize();

		// size of all the vertices for a tail
		const uint32 tailVSize = vSize * (_TailNbSeg + 1);
		
		// offset of the color in vertices
		const uint32 colorOff = _Vb.getColorOff();

		// offset for the head in a tail (the last vertex)
		const uint32 headOffset = vSize * _TailNbSeg;

		// address of the firstVertex

		uint8 *firstVertex = (uint8 *) _Vb.getVertexCoordPointer();

		// loop counters
		uint32 k, l;

		// pointer to the current vertex
		uint8 *currVertex;
		_Owner->incrementNbDrawnParticles(size); // for benchmark purpose	


		TPSAttribVector::const_iterator posIt, endPosIt = _Owner->getPos().end();
			
		// decal the whole vertex buffer (but not the last vertex)
		
		// TODO : cache optimization

		if (_ColorScheme && _ColorFading)
		{

			// the first color is set to black
			// other are modulated by the ratio of the intensity n and the intensity n + 1
			
			// the brightness increase between each segment in the tail
			const float lumiStep = 1.f / _TailNbSeg;
			float lumi = lumiStep;

			// set the first vertex to black				
			currVertex = firstVertex;
			for (k = 0; k < size; ++k)
			{
				CHECK_VERTEX_BUFFER(_Vb, currVertex + colorOff);
				CHECK_VERTEX_BUFFER(_Vb, currVertex);


				*(CRGBA *) (currVertex + colorOff) = CRGBA::Black;
				// copy the next vertex pos
				*(CVector *) currVertex = *(CVector *) (currVertex + vSize);
				currVertex += tailVSize ;			
			}

			for (l = 1; l < _TailNbSeg; ++l)
			{
				currVertex = firstVertex + l * vSize;
				uint8 ratio = (uint8) (255.0f * lumi / (lumi +  lumiStep));
				lumi += lumiStep;			
				for (k = 0; k < size; ++k)
				{
					CHECK_VERTEX_BUFFER(_Vb, currVertex + colorOff);
					CHECK_VERTEX_BUFFER(_Vb, currVertex + vSize + colorOff);
					// get the color of the next vertex and modulate
					((CRGBA *) (currVertex + colorOff))->modulateFromui(*(CRGBA *) (currVertex + vSize + colorOff), ratio);					

					// copy the next vertex pos
					CHECK_VERTEX_BUFFER(_Vb, currVertex);
					*(CVector *) currVertex = *(CVector *) (currVertex + vSize);
					currVertex += tailVSize ;			
				}
				
			}			
		}
		else if ( !_ColorScheme || _ColorFading)
		{		
			// we just decal the pos
			// we copy some extra pos, but we avoid 2 nested loops
			const uint32 nbVerts = (size * (_TailNbSeg + 1)) - 1;
			const uint8 *lastVert = firstVertex + nbVerts * vSize;

			for (currVertex = firstVertex; currVertex != lastVert; )
			{
				CHECK_VERTEX_BUFFER(_Vb, currVertex);
				*(CVector *) currVertex = *(CVector *) (currVertex + vSize);			
				currVertex += vSize;
			}	
		}
		else
		{		
			// we just copy the colors and the position
			// memcpy will copy some extra bytes, but it should remains fasters that 2 nested loops
			memcpy(firstVertex, firstVertex + vSize, size * tailVSize  - vSize); 		
		}

		// we fill the head of particles with the right color
		// With constant color, the setup was done in setupColor
		if (_ColorScheme)
		{
			_ColorScheme->make(_Owner, 0,  firstVertex + headOffset + colorOff
								, tailVSize, _Owner->getSize());		
		}

		currVertex = firstVertex + headOffset;
		// If we are in the same basis than the located that hold us, we can copy coordinate directly
		if (_SystemBasisEnabled == _Owner->isInSystemBasis())
		{	
			for (posIt = _Owner->getPos().begin(); posIt != endPosIt; ++posIt)
			{
				CHECK_VERTEX_BUFFER(_Vb, currVertex);
				*(CVector *) currVertex = *posIt;
				currVertex += tailVSize;
			}
		}
		else
		{
			// the tail are not in the same basis, we need a conversion matrix
			const CMatrix &m = _SystemBasisEnabled ?   /*_Owner->getOwner()->*/getInvertedSysMat() 
													 : /*_Owner->getOwner()->*/getSysMat();
			// copy the position after transformation
			for (posIt = _Owner->getPos().begin(); posIt != endPosIt; ++posIt)
			{
				CHECK_VERTEX_BUFFER(_Vb, currVertex);
				*(CVector *) currVertex = m * (*posIt);
				currVertex += tailVSize;
			}
		}
	}
}

///==========================================================================================
void CPSTailDot::draw(bool opaque)
{
	const uint32 size = _Owner->getSize();
	if (!size) return;		

	/// update material color
	CParticleSystem &ps = *(_Owner->getOwner());	

	IDriver *driver = getDriver();
	if (_SystemBasisEnabled)
	{
		driver->setupModelMatrix(/*_Owner->getOwner()->*/getSysMat());
	}
	else
	{
		driver->setupModelMatrix(CMatrix::Identity);
	}	
	driver->activeVertexBuffer(_Vb);		
	_Pb.setNumLine(size * _TailNbSeg);
	driver->render(_Pb, _Mat);

	PARTICLES_CHECK_MEM
}

///==========================================================================================
void CPSTailDot::resize(uint32 size)
{	
	nlassert(size < (1 << 16));
	resizeColor(size);
	resizeVb(_TailNbSeg, size);	
}

///==========================================================================================
void CPSTailDot::resizeVb(uint32 oldTailSize, uint32 size)
{
	
	if (!_Owner) return; // no attachement occured yet ...

	// calculate the primitive block : we got lines
		
	uint32 k, l;

	_Pb.reserveLine(_TailNbSeg * size);

	uint32 currIndex = 0;
	uint lineIndex = 0;

	for (k = 0; k < size; ++k)
	{
		for (l = 0; l < _TailNbSeg; ++l)
		{
			_Pb.setLine(lineIndex , currIndex + l, currIndex + l + 1);		
			++lineIndex;
		}

		currIndex += _TailNbSeg + 1;		
	}

	const uint32 oldSize = _Owner->getSize(); // number of used particles
	

	_Vb.setVertexFormat(CVertexBuffer::PrimaryColorFlag | CVertexBuffer::PositionFlag);
	_Vb.setNumVertices(size * (_TailNbSeg + 1));

	// fill the vertex buffer

	// particle that were present before
	// their old positions are copied

	// if the tail are not in the same basis, we need a conversion matrix

	const CMatrix *m;

	if (_SystemBasisEnabled == _Owner->isInSystemBasis())
	{
		m = &CMatrix::Identity;
	}
	else
	{
		m = _SystemBasisEnabled ?  & /*_Owner->getOwner()->*/getInvertedSysMat() 
								: &/*_Owner->getOwner()->*/getSysMat();
	}

	const TPSAttribVector &oldPos = _Owner->getPos();
	currIndex = 0;
	CVector currPos;

	for (k = 0; k < _Owner->getSize(); ++k)
	{
		currPos = *m * oldPos[k];
		for (l = 0; l <= _TailNbSeg; ++l)
		{
			_Vb.setVertexCoord( currIndex + l , currPos);			
		}
		currIndex += _TailNbSeg + 1;		
	}	
	// we don't need to setup the following vertices coordinates, they will be filled with newElement anyway...
	setupColor();	
}

///==========================================================================================
void CPSTailDot::setupColor(void)
{
	if (_Owner)
	{
		// we setup the WHOLE vb so, we need to get the max number of particles
		const uint32 size = _Owner->getMaxSize();	

		// size of the vertices
		const uint32 vSize = _Vb.getVertexSize();

		// offset of the color in vertices
		const uint32 colorOff = _Vb.getColorOff();

		// first vertex
		uint8 *firstVertex = (uint8 *) _Vb.getVertexCoordPointer();
		
		// point the current vertex color
		uint8 *currVertex =  firstVertex + colorOff;

		// loop counters
		uint k, l;

		if (_ColorScheme || !_ColorFading)
		{
			// we can't precompute the colors, so at first, we fill all with black
			const CRGBA &color = _ColorScheme ? CRGBA::Black : _Color;
			for (k = 0; k < size * (_TailNbSeg + 1); ++k)
			{
					CHECK_VERTEX_BUFFER(_Vb, currVertex);
					*(CRGBA *) currVertex = color;
					currVertex += vSize;
			}
		}
		else // constant color with color fading
		{
			const float lumiStep = 255.0f / _TailNbSeg;
			const  uint32 tailVSize = vSize * (_TailNbSeg + 1); // size of a whole tail in the vertex buffer
			float lumi = 0.f;
			for (l = 0; l <= _TailNbSeg; ++l)
			{
				currVertex = firstVertex + l * vSize + colorOff;
				CRGBA col;
				col.modulateFromui(_Color, (uint8) lumi);
				lumi += lumiStep;			
				for (k = 0; k < size; ++k)
				{				
					CHECK_VERTEX_BUFFER(_Vb, currVertex);
					*(CRGBA *)currVertex = col;				
					currVertex += tailVSize ;			
				}				
			}
		}
	}	
}

///==========================================================================================
void CPSTailDot::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	
	newColorElement(emitterLocated, emitterIndex);
	// if we got a constant color, everything has been setupped before	
	const uint32 index = _Owner->getNewElementIndex(); // the index of the element to be created
	const uint32 vSize =_Vb.getVertexSize();	// vertex size
	uint8 *currVert = (uint8 *) _Vb.getVertexCoordPointer() + (index * (_TailNbSeg + 1)) * vSize; // 

	const CVector &pos = _Owner->getPos()[index];	
	if (_ColorScheme)
	{
		// check wether the located is in the same basis than the tail
		// Otherwise, a conversion is required for the pos
		if (_SystemBasisEnabled == _Owner->isInSystemBasis())
		{
			for (uint32 k = 0; k < _TailNbSeg + 1; ++k)
			{
				CHECK_VERTEX_BUFFER(_Vb, currVert);
				CHECK_VERTEX_BUFFER(_Vb, currVert + _Vb.getColorOff());

				*(CVector *) currVert = pos;
				*(CRGBA *) (currVert + _Vb.getColorOff()) = CRGBA::Black;

				currVert += vSize; // go to next vertex
			}
		}
		else
		{
			const CMatrix &m = _SystemBasisEnabled ?   /*_Owner->getOwner()->*/getInvertedSysMat() 
											 : /*_Owner->getOwner()->*/getSysMat();
			for (uint32 k = 0; k < _TailNbSeg + 1; ++k)
			{
				CHECK_VERTEX_BUFFER(_Vb, currVert);
				*(CVector *) currVert = m * pos;
				CHECK_VERTEX_BUFFER(_Vb, currVert + _Vb.getColorOff());
				*(CRGBA *) (currVert + _Vb.getColorOff()) = CRGBA::Black;

				currVert += vSize; // go to next vertex
			}
		}

	}
	else
	{
		// color setup was done during setupColor()...

		// check wether the located is in the same basis than the tail
		// Otherwise, a conversion is required for the pos
		if (_SystemBasisEnabled == _Owner->isInSystemBasis())
		{
			for (uint32 k = 0; k < _TailNbSeg + 1; ++k)
			{
				CHECK_VERTEX_BUFFER(_Vb, currVert);
				*(CVector *) currVert = pos;			
				currVert += vSize; // go to next vertex
			}
		}
		else
		{
			const CMatrix &m = _SystemBasisEnabled ?  /*_Owner->getOwner()->*/getInvertedSysMat() 
											 : /*_Owner->getOwner()->*/getSysMat();
			for (uint32 k = 0; k < _TailNbSeg + 1; ++k)
			{
				CHECK_VERTEX_BUFFER(_Vb, currVert);
				*(CVector *) currVert = m * pos;			
				currVert += vSize; // go to next vertex
			}
		}
	}
}
		
///==========================================================================================
void CPSTailDot::deleteElement(uint32 index)
{
	deleteColorElement(index);
	// we copy the last element datas to this one data
	
	// vertex size
	const uint32 vSize =_Vb.getVertexSize();

	// source
	const uint8 *currSrcVert = (uint8 *) _Vb.getVertexCoordPointer() + ((_Owner->getSize() - 1) * (_TailNbSeg + 1)) * vSize;

	// destination
	uint8 *currDestVert = (uint8 *) _Vb.getVertexCoordPointer() + (index * (_TailNbSeg + 1)) * vSize;

	// copy the vertices
	memcpy(currDestVert, currSrcVert, vSize * (_TailNbSeg + 1) );	
}

///==========================================================================================
void CPSTailDot::setTailNbSeg(uint32 nbSeg)
{
	nlassert(nbSeg <= 255);
	uint32 oldTailSize = _TailNbSeg;
	_TailNbSeg = nbSeg;

	if (_Owner)
	{
		resizeVb(nbSeg, _Owner->getMaxSize());
	}
	notifyOwnerMaxNumFacesChanged();
}

///==========================================================================================
void CPSTailDot::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	const uint oldTailNbSeg = _TailNbSeg;

	f.serialVersion(1);
	CPSParticle::serial(f);
	CPSColoredParticle::serialColorScheme(f);	
	f.serial(_TailNbSeg, _ColorFading, _SystemBasisEnabled);

	serialMaterial(f);

	// In this version we don't save the vb state, as we probably won't need it
	// we just rebuild the vb
	
	if (f.isReading())
	{
		resizeVb(oldTailNbSeg, _Owner->getMaxSize());
		// init();		
	}		
}

///==========================================================================================
void CPSTailDot::updateMatAndVbForColor(void)
{
	setupColor();
}


} // NL3D
