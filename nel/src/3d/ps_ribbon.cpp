/** \file ps_ribbon.cpp
 * Ribbons particles.
 *
 * $Id: ps_ribbon.cpp,v 1.2 2002/02/20 11:19:23 vizerie Exp $
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

#include "3d/ps_ribbon.h"
#include "3d/ps_macro.h"
#include "3d/particle_system.h"
#include "3d/driver.h"
#include "nel/misc/matrix.h"

namespace NL3D 
{


using NLMISC::CMatrix;

///////////////////////////
// ribbon implementation //
///////////////////////////

	// predifined shapes
const CVector CPSRibbon::Triangle[] = { CVector (0, 1, 0),
								 CVector (1, -1, 0),
								 CVector (-1, -1, 0),								 
								};

const CVector CPSRibbon::Losange[] = { CVector (0, 1.f, 0),
								 CVector (1.f, 0, 0),
								 CVector (0, -1.f, 0),
								 CVector (-1.f, 0, 0)
								};

const CVector  CPSRibbon::HeightSides[] = {  CVector(-0.5f, 1, 0)
											,CVector(0.5f, 1, 0)
											,CVector(1, 0.5f, 0)
											,CVector(1, -0.5f, 0)
											,CVector(0.5f, -1, 0)
											,CVector(-0.5f, -1, 0)
											,CVector(-1, -0.5f, 0)
											,CVector(-1, 0.5f, 0) 
										 };


const CVector CPSRibbon::Pentagram[] = { CVector(0, 1, 0), CVector(1, -1, 0)
										 , CVector(-1, 0, 0), CVector(1, 0, 0)
										 , CVector(-1, -1, 0)
										};

const uint32 CPSRibbon::NbVerticesInTriangle = sizeof(CPSRibbon::Triangle) / sizeof(CVector);
const uint32 CPSRibbon::NbVerticesInLosange = sizeof(Losange) / sizeof(CVector);
const uint32 CPSRibbon::NbVerticesInHeightSide = sizeof(CPSRibbon::HeightSides) / sizeof(CVector);
const uint32 CPSRibbon::NbVerticesInPentagram = sizeof(CPSRibbon::Pentagram) / sizeof(CVector);



///==================================================================================================================
CPSRibbon::CPSRibbon(uint32 nbSegmentInTail
				  , const CVector *shape
				  , uint32 nbPointsInShape) 
				  : _ColorFading(true)
				  ,	_SystemBasisEnabled(false)	 
				  , _DyingRibbons(NULL)
				  , _ShapeNbSeg(0)
				  , _TailNbSeg(4)
				  , _NbDyingRibbons(0)
				  , _UFactor(1.f)
				  , _VFactor(1.f)
{

	init();
	nlassert(_TailNbSeg >= 2);
	setShape(shape, nbPointsInShape);
	setTailNbSeg(nbSegmentInTail); 	
	_Name = std::string("Ribbon");
}


///==================================================================================================================
uint32 CPSRibbon::getMaxNumFaces(void) const
{
	nlassert(_Owner);
	return (_Owner->getMaxSize() * _TailNbSeg * _ShapeNbSeg) << 1;
}


///==================================================================================================================
CPSRibbon::~CPSRibbon()
{		
	delete _DyingRibbons;
}

///==================================================================================================================
bool CPSRibbon::hasTransparentFaces(void)
{
	return getBlendingMode() != CPSMaterial::alphaTest ;
}

///==================================================================================================================
bool CPSRibbon::hasOpaqueFaces(void)
{
	return !hasTransparentFaces();
}

///==================================================================================================================
void CPSRibbon::setPersistAfterDeath(bool persist)
{
	if (persist)
	{
		if (_DyingRibbons) return;
		_DyingRibbons = new CRibbonsDesc;		
		if (_Owner)
		{
			_DyingRibbonsLifeLeft.resize(_Owner->getMaxSize());
		}
		_NbDyingRibbons = 0;
		init(*_DyingRibbons);
		setupVertexFormatNMat(*_DyingRibbons);
		resizeVb(*_DyingRibbons);
	}
	else
	{		
		delete _DyingRibbons;
		_DyingRibbons = NULL;	
	}
}


///==================================================================================================================
void CPSRibbon::setTailNbSeg(uint32 nbSeg)
{	
	
	nlassert(nbSeg >= 2);
	_TailNbSeg = nbSeg;
	resizeVb(_AliveRibbons);	
	if (_DyingRibbons)
	{
		resizeVb(*_DyingRibbons);
	}
	// compute the ratio tab
	_SliceRatioTab.resize(_TailNbSeg);
	const float radiusStep = 1.f / _TailNbSeg;
	float radius = 0.f;
	for (std::vector<float>::iterator it = _SliceRatioTab.begin(); it != _SliceRatioTab.end(); ++it)
	{
		*it = radius / (radius + radiusStep);
		radius += radiusStep;
	}
	notifyOwnerMaxNumFacesChanged();	
}

///==================================================================================================================
void CPSRibbon::setShape(const CVector *shape, uint32 nbPointsInShape)
{
	nlassert(nbPointsInShape >= 3);
	
	if (nbPointsInShape != _ShapeNbSeg)
	{
		_ShapeNbSeg = nbPointsInShape;
		_Shape.resize(nbPointsInShape);

		std::copy(shape, shape + nbPointsInShape, _Shape.begin());
		resizeVb(_AliveRibbons);
		if (_DyingRibbons)
		{
			resizeVb(*_DyingRibbons);
		}
		notifyOwnerMaxNumFacesChanged();
	}
	else
	{
		std::copy(shape, shape + nbPointsInShape, _Shape.begin());
	}		
}

///==================================================================================================================
void CPSRibbon::getShape(CVector *shape) const
{
	std::copy(_Shape.begin(), _Shape.end(), shape);
}

///==================================================================================================================
void CPSRibbon::init(void)
{
	_Mat.setLighting(false);	
	_Mat.setZFunc(CMaterial::less);
	_Mat.setDoubleSided(true);
	
	init(_AliveRibbons);
	if (_DyingRibbons)
	{
		init(*_DyingRibbons);
	}
}

///==================================================================================================================
void CPSRibbon::init(CRibbonsDesc &rb)
{
	rb._Vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag );	// no texture is the default
}

///==================================================================================================================
void CPSRibbon::decalRibbons(CRibbonsDesc &rb, const uint32 size)	
{
	if (!size) return;

	// size of the vertices
	const uint32 vSize = rb._Vb.getVertexSize();

	// size of a shape in bytes
	const uint32 sSize = vSize * _ShapeNbSeg;

	// size of all the vertices for a tail
	const uint32 tailVSize = sSize * (_TailNbSeg + 1);	
	
	// offset of the color in vertices
	const uint32 colorOff = rb._Vb.getColorOff();

	// address of the firstVertex

	uint8 *firstVertex = (uint8 *) rb._Vb.getVertexCoordPointer();

	// loop counters
	uint32 k, l;

	// pointer to the current vertex
	uint8 *currVertex;	

	// during the first pass, we resize each 'slice' in ribbons, and we decal them
	// the first slice is centered to the tail center

	// iterator on the center pos of each slice
	std::vector<CVector>::iterator posIt = rb._Pos.begin();
		
	
	// Iterator on each slice color, used to store slice color when fading is applied
	std::vector<CRGBA>::iterator colIt;


	if (!_ColorScheme)
	{
		// we use colors in the vertex buffer as it
		currVertex = firstVertex;
		for (k = 0; k < size; ++k)
		{		
			std::vector<float>::const_iterator ratioIt = _SliceRatioTab.begin();
			for (; ratioIt != _SliceRatioTab.end(); ++ratioIt)
			{
				// decal the center of the slice
				
				*posIt = *(posIt + 1);
				for (l = 0; l < _ShapeNbSeg; ++l)
				{
					CHECK_VERTEX_BUFFER(rb._Vb, currVertex);
					CHECK_VERTEX_BUFFER(rb._Vb, currVertex + sSize);

					CVector &v = *(CVector *) currVertex;				
					// points the same vertex in the next slice in the current ribbon
					const CVector &vNext = *(CVector *) (currVertex + sSize);				

					// rescale the point
					//v = *posIt + *ratioIt * (vNext - *posIt);

					v.x = posIt->x + *ratioIt * (vNext.x - posIt->x);
					v.y = posIt->y + *ratioIt * (vNext.y - posIt->y);
					v.z = posIt->z + *ratioIt * (vNext.z - posIt->z);

					// points next vertex in current slice
					currVertex += vSize;
				}
				++posIt;

			}
			// we didn't process the last slice
			currVertex += sSize;
			++posIt;
		}
	}
	else
	{
		if (!_ColorFading)
		{
			// we just decal colors in the vertex buffer
			currVertex = firstVertex;
			for (k = 0; k < size; ++k)
			{		
				std::vector<float>::const_iterator ratioIt = _SliceRatioTab.begin();
				for (; ratioIt != _SliceRatioTab.end(); ++ratioIt)
				{
					// decal the center of the slice

					CHECK_VERTEX_BUFFER(rb._Vb, currVertex + sSize + colorOff);
					const CRGBA nextCol = *(CRGBA *) (currVertex + sSize + colorOff);
					
					*posIt = *(posIt + 1);
					for (l = 0; l < _ShapeNbSeg; ++l)
					{
						CHECK_VERTEX_BUFFER(rb._Vb, currVertex);
						CHECK_VERTEX_BUFFER(rb._Vb, currVertex + sSize);

						CVector &v = *(CVector *) currVertex;				
						// points the same vertex in the next slice in the current ribbon
						const CVector &vNext = *(CVector *) (currVertex + sSize);				

						// rescale the point
						v = *posIt + *ratioIt * (vNext - *posIt);

						// decal color

						CHECK_VERTEX_BUFFER(rb._Vb, currVertex + colorOff);
						*(CRGBA *) (currVertex + colorOff) = nextCol ;

						// points next vertex in current slice
						currVertex += vSize;
					}
					++posIt;
				}
				// we didn't process the last slice
				currVertex += sSize;
				++posIt;
			}
		}
		else
		{
			// we must apply color fading allong the ribbon
			colIt = rb._ColTab.begin();
			currVertex = firstVertex;
			const float colorRatioStep = 255.f / (_TailNbSeg + 1);
			for (k = 0; k < size; ++k)
			{		
				std::vector<float>::const_iterator ratioIt = _SliceRatioTab.begin();
				float currColRatio = 0.f;
				CRGBA sliceCol;
				for (; ratioIt != _SliceRatioTab.end(); ++ratioIt)
				{
					// decal the center of the slice
					
					*colIt = *(colIt + 1); // decal colors
					sliceCol.modulateFromui(*colIt, (uint) currColRatio);

					*posIt = *(posIt + 1);
					for (l = 0; l < _ShapeNbSeg; ++l)
					{
						CHECK_VERTEX_BUFFER(rb._Vb, currVertex);
						CHECK_VERTEX_BUFFER(rb._Vb, currVertex + colorOff);
						CHECK_VERTEX_BUFFER(rb._Vb, currVertex + sSize);

						CVector &v = *(CVector *) currVertex;				
						// points the same vertex in the next slice in the current ribbon
						const CVector &vNext = *(CVector *) (currVertex + sSize);				

						// rescale the point
						v = *posIt + *ratioIt * (vNext - *posIt);

						// copy color
						*(CRGBA *) (currVertex + colorOff) = sliceCol ;

						// points next vertex in current slice
						currVertex += vSize;
					}

					currColRatio += colorRatioStep;

					++colIt;
					++posIt;
				}
				// we didn't process the last slice
				currVertex += sSize;
				++posIt;
				++colIt;
			}
		}
	}
}


///==================================================================================================================
void CPSRibbon::computeLastSlice(CRibbonsDesc &rb, const uint32 size)
{	
	if (!size) return;

	// size of the vertices
	const uint32 vSize = rb._Vb.getVertexSize();

	// size of a shape in bytes
	const uint32 sSize = vSize * _ShapeNbSeg;

	// size of all the vertices for a tail
	const uint32 tailVSize = sSize * (_TailNbSeg + 1);

		
	// offset of the color in vertices
	const uint32 colorOff = rb._Vb.getColorOff();

	// address of the firstVertex

	uint8 *firstVertex = (uint8 *) rb._Vb.getVertexCoordPointer();

	// loop counters
	uint32 k, l;

	// pointer to the current vertex
	uint8 *currSliceFirstVertex, *currVertex;

	// iterator on speed
	// it is used to compute a basis for the shape to generate
	TPSAttribVector::const_iterator speedIt = _Owner->getSpeed().begin();

	// iterator on the center pos of each slice
	std::vector<CVector>::iterator posIt;

	// Iterator on each slice color, used to store slice color when fading is applied
	std::vector<CRGBA>::iterator colIt;


	// now, create the last slice for each ribbon
	posIt = rb._Pos.begin() + _TailNbSeg;

	TPSAttribVector::const_iterator  srcPosIt = _Owner->getPos().begin();
	
	// points the first vertex of the last slice in the first ribbon
	currSliceFirstVertex = firstVertex + sSize * _TailNbSeg;

	CMatrix shapeMat;
	std::vector<CVector>::const_iterator shapeIt, shapeEnd = _Shape.end();
	if (_ColorFading && _ColorScheme)
	{
		 colIt = rb._ColTab.begin() + _TailNbSeg ;
	}

	k = 0;

	// check wether we are in the system or world basis

	
	CMatrix modelMat, rotModelMat;

	if (_Owner->isInSystemBasis() == _SystemBasisEnabled)
	{
		modelMat = CMatrix::Identity;
	}
	else
	{
		modelMat = _SystemBasisEnabled ?   /*_Owner->getOwner()->*/getInvertedSysMat() 
								:          /*_Owner->getOwner()->*/getSysMat();
	}

	rotModelMat = modelMat;
	rotModelMat.setPos(CVector::Null);

	 

	for (;;)
	{
				
		*posIt = modelMat * *srcPosIt;
		

		// compute a basis where to put the shape in
		shapeMat = rotModelMat * CPSUtil::buildSchmidtBasis(*speedIt);		


		currVertex = currSliceFirstVertex;	

		// compute the angles and 2x2 rot matrix, depending on the attributes

		const float scale = _SizeScheme ? _SizeScheme->get(_Owner, k) : _ParticleSize;
		const float ca = CPSUtil::getCos(	
											(sint32)
											(
												_Angle2DScheme ?
												_Angle2DScheme->get(_Owner, k) : _Angle2D
											)
										);

		const float sa = CPSUtil::getSin(
											(sint32)
											(
												_Angle2DScheme ?
												_Angle2DScheme->get(_Owner, k) : _Angle2D
											)
										);


		for (shapeIt = _Shape.begin(); shapeIt != shapeEnd; ++shapeIt )
		{
				
			const CVector &v = *shapeIt;
			CHECK_VERTEX_BUFFER(rb._Vb, currVertex);
			*(CVector *) currVertex = *posIt + scale * (shapeMat * CVector( ca * v.x - sa * v.y 
																			,sa * v.x + ca * v.y
																			,v.z));
			currVertex += vSize;
		}

		if (_ColorScheme)
		{			
			const CRGBA col = _ColorScheme->get(_Owner, k);
			currVertex -= sSize; // go back to the start of the slice
			for (l = 0; l < _ShapeNbSeg; ++l)
			{
				CHECK_VERTEX_BUFFER(rb._Vb, currVertex + colorOff);
				*(CRGBA *) (currVertex + colorOff) = col;
				currVertex += vSize;
			}
			if (_ColorFading)
			{
				*colIt = col;			
			}
		}
		// go to the first vertex of the last slice in the next ribbon
		currSliceFirstVertex += tailVSize;

		++k;

		if (k == size) break;

		++speedIt;		
		posIt += _TailNbSeg + 1;
		++srcPosIt;

		if (_ColorScheme && _ColorFading)
		{		
			colIt += _TailNbSeg + 1;
		}
	}
}

///==================================================================================================================
void CPSRibbon::render(CRibbonsDesc &rb, const uint32 size)
{
	IDriver *driver = getDriver();	
	driver->activeVertexBuffer(rb._Vb);		
	driver->renderTriangles(_Mat, &rb._Ib[0], 2 * size * _TailNbSeg * _ShapeNbSeg);		
}

///==================================================================================================================
void CPSRibbon::step(TPSProcessPass pass, TAnimationTime ellapsedTime, TAnimationTime realEt)
{	
	if (pass == PSMotion)
	{
		const uint32 size = _Owner->getSize();		
		decalRibbons(_AliveRibbons, size);
		computeLastSlice(_AliveRibbons, size);
		if (_DyingRibbons)
		{
			decalRibbons(*_DyingRibbons, _NbDyingRibbons);
			render(*_DyingRibbons, _NbDyingRibbons);
		}
	}
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
}

///==================================================================================================================
void CPSRibbon::draw(bool opaque)
{		
	// we decal ribbon during the motion pass
	PARTICLES_CHECK_MEM;
	const uint32 size = _Owner->getSize();   
	if (size == 0) return;


	/// update material color
	CParticleSystem &ps = *(_Owner->getOwner());		
	if (ps.getColorAttenuationScheme() != NULL)
	{		
		CPSMaterial::forceModulateConstantColor(true, ps.getGlobalColor());		
	}
	else
	{	
		forceModulateConstantColor(false);
		_Mat.setColor(ps.getGlobalColor());
	}
	
	// now, draw the ribbons
	if (_SystemBasisEnabled)
	{
		getDriver()->setupModelMatrix(/*_Owner->getOwner()->*/getSysMat());
	}
	else
	{
		getDriver()->setupModelMatrix(CMatrix::Identity);
	}	
	render(_AliveRibbons, size);

	// deals with dying ribbons
	if (_DyingRibbons)
	{		
		// now, check for dead ribbons
		uint k = 0;
		for (;;)
		{
			if (k >= _NbDyingRibbons)
			{
				break;
			}	
			_DyingRibbonsLifeLeft[k] ++;
			if (_DyingRibbonsLifeLeft[k] > (_TailNbSeg + 1))
			{
				copyElement(*_DyingRibbons, _NbDyingRibbons - 1, *_DyingRibbons, k); // replace this ribbon with the last
				nlassert(_NbDyingRibbons);
				_NbDyingRibbons --;
				_DyingRibbonsLifeLeft[k] = _DyingRibbonsLifeLeft[_NbDyingRibbons ];
			}
			else
			{
				++k;
			}					
		}
	}

	PARTICLES_CHECK_MEM;
}

///==================================================================================================================
void CPSRibbon::resize(uint32 size)
{	
	nlassert(size < (1 << 16));
	resizeAngle2D(size);
	resizeSize(size);
	resizeColor(size);
	resizeVb(_AliveRibbons);	
	if (_DyingRibbons)
	{
		resizeVb(*_DyingRibbons);
		_DyingRibbonsLifeLeft.resize(size);
	}
}

///==================================================================================================================
void CPSRibbon::resizeVb(CRibbonsDesc &rb)
{	
	if (!_Owner) return; // no attachement occured yet ...
	const uint32 size = _Owner->getMaxSize();
	// some loop counters
	uint32 k, l, m;
	// setup the index buffer			
	rb._Ib.resize(6 * _ShapeNbSeg * _TailNbSeg * size); // 2 triangles of 6 index for each slice part
	std::vector<uint32>::iterator ptCurrIndex = rb._Ib.begin();
	uint32 currIndex = 0;
	for (k = 0; k < size; ++k)
	{
		for (l = 0; l < _TailNbSeg; ++l)
		{
			
			for (m = 0; m < _ShapeNbSeg - 1; ++m)
			{			
				*ptCurrIndex++ = currIndex;
				*ptCurrIndex++ = currIndex + _ShapeNbSeg;
				*ptCurrIndex++ = currIndex + 1;
										
				*ptCurrIndex++ = currIndex + 1;
				*ptCurrIndex++ = currIndex + _ShapeNbSeg;
				*ptCurrIndex++ = currIndex + _ShapeNbSeg + 1;				
				++currIndex;
			}

			// loop around the slice		
			*ptCurrIndex++ = currIndex;
			*ptCurrIndex++ = currIndex + _ShapeNbSeg;
			*ptCurrIndex++ = currIndex - _ShapeNbSeg + 1;		

			*ptCurrIndex++ = currIndex - _ShapeNbSeg + 1;
			*ptCurrIndex++ = currIndex + _ShapeNbSeg;
			*ptCurrIndex++ = currIndex + 1;

			++currIndex;

		}
		currIndex += _ShapeNbSeg;
	
	}
	
	// setup the vertex buffer and the pos vect
	rb._Pos.resize(size * (_TailNbSeg + 1));
	rb._Vb.setNumVertices(size * (_TailNbSeg + 1) * _ShapeNbSeg);
	TPSAttribVector::const_iterator posIt = _Owner->getPos().begin(), endPos = _Owner->getPos().end();	
	for ( k = 0; posIt != endPos; ++posIt, ++k)
	{
		initRibbonPos(rb, k, *posIt);
	}		

	setupColor(rb);
	setupUV(rb);
}

///==================================================================================================================
void CPSRibbon::initRibbonPos(CRibbonsDesc &rb, uint32 index, const CVector &aPos)
{
	// aPos is in the basis chosen for this particle (system or world)
	// we must expressed it in the tail basis
	CVector pos;
	if (_SystemBasisEnabled == _Owner->isInSystemBasis())
	{
		pos = aPos;
	}
	else
	{
		pos = _SystemBasisEnabled ?   getInvertedSysMat() * aPos 
								:     getSysMat()         * aPos;
	}	
	const uint32 vSize = rb._Vb.getVertexSize();
	uint8 *currVertex = ((uint8 *) rb._Vb.getVertexCoordPointer())
						+ index * vSize * (_TailNbSeg + 1 ) * _ShapeNbSeg;
	std::vector<CVector>::iterator itPos = rb._Pos.begin() + (index * (_TailNbSeg + 1) );	
	std::fill(itPos, itPos + _TailNbSeg + 1, pos);		
	// at start, all pos are the same, e.g centered on the particle
	for (uint32 k = 0; k < ((_TailNbSeg + 1) * _ShapeNbSeg); ++k)
	{
		CHECK_VERTEX_BUFFER(rb._Vb, currVertex);
		*(CVector *) currVertex = pos;		
		currVertex += vSize;
	}		
}

///==================================================================================================================
void CPSRibbon::setupColor(CRibbonsDesc &rb)
{
	if (!_Owner) return;
		// we setup the WHOLE vb so, we need to get the max number of particles
	const uint32 size = _Owner->getMaxSize();	

	// size of the vertices
	const uint32 vSize = rb._Vb.getVertexSize();

	// size of a slice
	const uint32 sSize = vSize * _ShapeNbSeg;

	// offset of the color in vertices
	const uint32 colorOff = rb._Vb.getColorOff();

	// first vertex
	uint8 *firstVertex = (uint8 *) rb._Vb.getVertexCoordPointer();
	
	// point the current vertex color
	uint8 *currVertex =  firstVertex + colorOff;

	// loop counters
	uint k, l, m;
	
	if (_ColorScheme && _ColorFading)
	{
		// we must keep colors in a table in order to blend them correctly
		// If there's no color fading, color are just decaled

		rb._ColTab.resize(size * (_TailNbSeg + 1) );
		std::fill(rb._ColTab.begin(), rb._ColTab.end(), CRGBA::Black);		
	}
	else
	{
		if (rb._ColTab.size() != 0)
		{
			NLMISC::contReset(rb._ColTab);
		}
	}

	if (!_ColorFading)
	{
		
		for (k = 0; k < size * (_TailNbSeg + 1) * _ShapeNbSeg ; ++k)
		{
				CHECK_VERTEX_BUFFER(rb._Vb, currVertex);
				*(CRGBA *) currVertex = _Color;
				currVertex += vSize;
		}
	}
	else // constant color with color fading
	{
		const float lumiStep = 255.0f / _TailNbSeg;
		const  uint32 tailVSize = sSize * (_TailNbSeg + 1); // size of a whole tail in the vertex buffer
		float lumi = 0.f;
		for (l = 0; l <= _TailNbSeg; ++l)
		{
			currVertex = firstVertex + l * sSize + colorOff;
			CRGBA col;
			col.modulateFromui(_Color, (uint8) lumi);
			lumi += lumiStep;			
			for (k = 0; k < size; ++k)
			{	
				for (m = 0; m < _ShapeNbSeg; ++m)
				{
					CHECK_VERTEX_BUFFER(rb._Vb, currVertex);
					*(CRGBA *) currVertex = col;				
					currVertex += vSize;
				}
				currVertex += tailVSize  - sSize;			
			}
			
		}

	}
}

///==================================================================================================================
void CPSRibbon::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{		
	const uint32 index = _Owner->getNewElementIndex(); 
	initRibbonPos(_AliveRibbons, index, _Owner->getPos()[index] ); 
	newColorElement(emitterLocated, emitterIndex);
	newSizeElement(emitterLocated, emitterIndex);
	newAngle2DElement(emitterLocated, emitterIndex);
}
		
///==================================================================================================================
void CPSRibbon::deleteElement(uint32 index)
{
	deleteColorElement(index);
	deleteSizeElement(index);
	deleteAngle2DElement(index);
	// if the light must persist after death, we must copy it in the dying ribbon tab
	if (_DyingRibbons)
	{
		nlassert(_NbDyingRibbons != _Owner->getMaxSize());
		copyElement(_AliveRibbons, index, *_DyingRibbons, _NbDyingRibbons);
		_DyingRibbonsLifeLeft[_NbDyingRibbons] = 0;
		// now, we setup the last slice of the ribbon so that all points are centered
		const CVector &center = _DyingRibbons->_Pos[_NbDyingRibbons * (_TailNbSeg + 1) + _TailNbSeg];
		const uint32 vSize = _DyingRibbons->_Vb.getVertexSize(); // vertex size
		const uint32 sSize = vSize * _ShapeNbSeg;				// slice size
		const uint32 tSize = sSize * (_TailNbSeg + 1); // tail size
		const uint32 colorOff = _DyingRibbons->_Vb.getColorOff(); // offset to color
		// points the first vertex of the last slice in the ribbon that we copied
		uint8 *currVertex = (uint8 *) _DyingRibbons->_Vb.getVertexCoordPointer() 
						  + tSize * _NbDyingRibbons
						  + sSize * _TailNbSeg;
		// color must be set to black if there's a color scheme, and no color fading
		// otherwise, the color remains unchanged
		CHECK_VERTEX_BUFFER(_DyingRibbons->_Vb, currVertex + colorOff);
		const CRGBA col = _ColorScheme && !_ColorFading ? CRGBA::Black : *(CRGBA *) (currVertex + colorOff);
		for (uint32 k = 0; k < _ShapeNbSeg; ++k)
		{
			CHECK_VERTEX_BUFFER(_DyingRibbons->_Vb, currVertex);
			CHECK_VERTEX_BUFFER(_DyingRibbons->_Vb, currVertex + colorOff);

			*(CVector *) currVertex = center;
			*(CRGBA *) (currVertex + colorOff) = col;
			currVertex += vSize;
		}
		// if there's color fading and a color scheme, color is stored in _ColTab...
		if (_ColorScheme && _ColorFading)
		{
			_DyingRibbons->_ColTab[_NbDyingRibbons * (_TailNbSeg + 1) + _TailNbSeg] = CRGBA::Black;
		}		
		++_NbDyingRibbons;
	}
	deleteElement(_AliveRibbons, index, _Owner->getSize());
}

///==================================================================================================================
void CPSRibbon::deleteElement(CRibbonsDesc &rb, uint32 index, const uint32 size)
{
	// copy the last element over this own
	copyElement(rb, size - 1 , rb, index);
}

///==================================================================================================================
void CPSRibbon::copyElement(CRibbonsDesc &rbSrc, uint32 srcIndex, CRibbonsDesc &rbDest, uint32 destIndex)
{
	if 	(&rbSrc == &rbDest && destIndex == srcIndex) return;
	// we copy the last element datas to this one data	
	// size of a ribbon in byte
	const uint32 ts = (_TailNbSeg + 1);
	const uint32 rSize = rbSrc._Vb.getVertexSize() * ts * _ShapeNbSeg;
	// destination
	uint8 *destVert = (uint8 *) rbDest._Vb.getVertexCoordPointer() + destIndex * rSize;
	// source
	const uint8 *srcVert = (uint8 *) rbSrc._Vb.getVertexCoordPointer() + srcIndex * rSize;
	// copy the vertices
	memcpy(destVert, srcVert, rSize );	
	// copy the pos tab;
	const std::vector<CVector>::iterator  destStart = rbDest._Pos.begin();
	const std::vector<CVector>::const_iterator  srcStart = rbSrc._Pos.begin();	
	std::copy(srcStart + ts * srcIndex, srcStart + ts * (srcIndex + 1), destStart + ts * destIndex);
	// copy the color tab when present
	if (_ColorScheme && _ColorFading)
	{
		const std::vector<CRGBA>::iterator  destColStart = rbDest._ColTab.begin();	
		const std::vector<CRGBA>::const_iterator  srcColStart = rbSrc._ColTab.begin();	
		std::copy(srcColStart + ts * srcIndex, srcColStart + ts * (srcIndex +1), destColStart + ts * destIndex);
	}
}

///==================================================================================================================
void CPSRibbon::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	f.serialVersion(1);
	CPSParticle::serial(f);
	CPSColoredParticle::serialColorScheme(f);	
	CPSSizedParticle::serialSizeScheme(f);
	CPSRotated2DParticle::serialAngle2DScheme(f);
	f.serial( _ColorFading, _SystemBasisEnabled);
	serialMaterial(f);

	// we don't save dying ribbon pos in this version
	bool drEnabled;
	ITexture *tex = NULL;
	if (!f.isReading())
	{
		drEnabled = (_DyingRibbons != NULL);
		f.serial(drEnabled);		
		f.serial(_TailNbSeg);
		tex = _Tex;
		f.serialPolyPtr(tex);
		if (_Tex)
		{
			f.serial(_UFactor, _VFactor) ;
		}
	}
	else
	{
		f.serial(drEnabled);
		setPersistAfterDeath(drEnabled);
		uint32 tnbs;
		f.serial(tnbs);
		setTailNbSeg(tnbs);
		f.serialPolyPtr(tex);
		_Tex = tex;				
		if (_Tex)
		{
			f.serial(_UFactor, _VFactor) ;
		}
	}	

	// shape serialization
	{
		f.serialCont(_Shape);
		_ShapeNbSeg = _Shape.size();
	}

	// In this version we don't save the vb state, as we probably won't need it
	// we just rebuild the vb	
	if (f.isReading())
	{
		setupVertexFormatNMat(_AliveRibbons);
		resizeVb(_AliveRibbons);
		if (_DyingRibbons)
		{
			setupVertexFormatNMat(*_DyingRibbons);
			resizeVb(*_DyingRibbons);
		}		
	}		
}

///==================================================================================================================
void CPSRibbon::updateMatAndVbForColor(void)
{	
	setupColor(_AliveRibbons);
	if (_DyingRibbons)
	{
			setupColor(*_DyingRibbons);
	}
}

///==================================================================================================================
void CPSRibbon::updateMatAndVbForTexture(void)
{
	setupVertexFormatNMat(_AliveRibbons);	
	if (_DyingRibbons)
	{
		setupVertexFormatNMat(*_DyingRibbons);
	}		
	if (_Owner) 
	{		
		resizeVb(_AliveRibbons);
		if (_DyingRibbons)
		{
			resizeVb(*_DyingRibbons);
		}
	}	
}

///==================================================================================================================
void  CPSRibbon::setupVertexFormatNMat(CRibbonsDesc &rb)
{
	if (_Tex == NULL) // no texture needed
	{
		rb._Vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag );		
	}
	else
	{
		rb._Vb.setVertexFormat(CVertexBuffer::PositionFlag | CVertexBuffer::PrimaryColorFlag | CVertexBuffer::TexCoord0Flag );	
		_Mat.setTexture(0, _Tex);
	}
}

///==================================================================================================================
void CPSRibbon::setupUV(CRibbonsDesc &rb)
{
	if (_Tex)
	{
		uint32 size = _Owner->getMaxSize(); 
		uint8 *currV = (uint8 *) rb._Vb.getVertexCoordPointer();
		// now, setup the uv's				
		for (uint32 k = 0; k < size; ++k)
		{
			for (uint32 l = 0; l <= _TailNbSeg; ++l)
			{
				for (uint32 m = 0; m < _ShapeNbSeg; ++m)
				{
					*(CUV *) (currV + rb._Vb.getTexCoordOff()) = CUV(_UFactor * m / float(_ShapeNbSeg)
															   , _VFactor * l / float(_TailNbSeg + 1));
					currV += rb._Vb.getVertexSize();
				}				
			}
		}
	}
}


} // NL3D
