/** \file ps_ribbon.cpp
 * Ribbons particles.
 *
 * $Id: ps_ribbon.cpp,v 1.4 2002/02/28 12:59:51 besson Exp $
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

#include "3d/ps_ribbon.h"
#include "3d/ps_macro.h"
#include "3d/particle_system.h"
#include "3d/driver.h"
#include "3d/ps_util.h"
#include "3d/texture_mem.h"
#include "nel/misc/matrix.h"

namespace NL3D 
{

static NLMISC::CRGBA GradientB2W[] = {NLMISC::CRGBA(0, 0, 0, 0), NLMISC::CRGBA(255, 255, 255, 255) };

/// private use : this create a gradient texture that goew from black to white
static ITexture *CreateGradientTexture()
{
	std::auto_ptr<CTextureMem> tex(new CTextureMem((uint8 *) &GradientB2W,
												   sizeof(GradientB2W),
												   false, /* dont delete */
												   false, /* not a file */
												   2, 1)
								  );
	tex->setWrapS(ITexture::Clamp);
	tex->setShareName("#GradBW");
	return tex.release();
}


///////////////////////////
// ribbon implementation //
///////////////////////////

	// predifined shapes
const NLMISC::CVector CPSRibbon::Triangle[] = 
{ 
	NLMISC::CVector(0, 1, 0),
	NLMISC::CVector(1, -1, 0),
	NLMISC::CVector(-1, -1, 0),								 
};

const NLMISC::CVector CPSRibbon::Losange[] = 
{ 
	NLMISC::CVector(0, 1.f, 0),
	NLMISC::CVector(1.f, 0, 0),
	NLMISC::CVector(0, -1.f, 0),
	NLMISC::CVector(-1.f, 0, 0)
};

const NLMISC::CVector  CPSRibbon::HeightSides[] = 
{  
	NLMISC::CVector(-0.5f, 1, 0),
	NLMISC::CVector(0.5f, 1, 0),
	NLMISC::CVector(1, 0.5f, 0),
	NLMISC::CVector(1, -0.5f, 0),
	NLMISC::CVector(0.5f, -1, 0),
	NLMISC::CVector(-0.5f, -1, 0),
	NLMISC::CVector(-1, -0.5f, 0),
	NLMISC::CVector(-1, 0.5f, 0) 
};


const NLMISC::CVector CPSRibbon::Pentagram[] = 
{ 
	NLMISC::CVector(0, 1, 0),
	NLMISC::CVector(1, -1, 0),
	NLMISC::CVector(-1, 0, 0),
	NLMISC::CVector(1, 0, 0),
	NLMISC::CVector(-1, -1, 0)
};

const uint CPSRibbon::NbVerticesInTriangle = sizeof(CPSRibbon::Triangle) / sizeof(CVector);
const uint CPSRibbon::NbVerticesInLosange = sizeof(Losange) / sizeof(CVector);
const uint CPSRibbon::NbVerticesInHeightSide = sizeof(CPSRibbon::HeightSides) / sizeof(CVector);
const uint CPSRibbon::NbVerticesInPentagram = sizeof(CPSRibbon::Pentagram) / sizeof(CVector);


CPSRibbon::TVBMap			CPSRibbon::_VBMap;			     // index / vertex buffers with no color
CPSRibbon::TVBMap			CPSRibbon::_FadedVBMap;			 // index / vertex buffers for constant color with fading
CPSRibbon::TVBMap			CPSRibbon::_ColoredVBMap;		 // index / vertex buffer + colors
CPSRibbon::TVBMap			CPSRibbon::_FadedColoredVBMap;   // index / vertex buffer + faded colors

CPSRibbon::TVBMap			CPSRibbon::_TexVBMap;			    // index / vertex buffers with no color + texture
CPSRibbon::TVBMap			CPSRibbon::_TexFadedVBMap;	        // index / vertex buffers for constant color with fading + texture
CPSRibbon::TVBMap			CPSRibbon::_TexColoredVBMap;		// index / vertex buffer + colors + texture
CPSRibbon::TVBMap			CPSRibbon::_TexFadedColoredVBMap;   // index / vertex buffer + faded colors + texture


///==================================================================================================================
void CPSRibbon::serial(NLMISC::IStream &f) throw(NLMISC::EStream)
{
	sint ver = f.serialVersion(2);
	if (ver == 1)
	{
		nlassert(f.isReading());

		/// we had CPSParticle::serial(f), but this is not the base class anymore, so we emulate this...
		/// version 2 : auto-lod saved
		sint ver2 = f.serialVersion(2);

		// here is CPSLocatedBindable::serial(f)
		sint ver3 = f.serialVersion(4);
		f.serialPtr(_Owner);
		if (ver3 > 1) f.serialEnum(_LOD);
		if (ver3 > 2) f.serial(_Name);
		if (ver3 > 3) 
		{
			if (f.isReading())
			{
				uint32 id;
				f.serial(id);
				setExternID(id);
			}
			else
			{
				f.serial(_ExternID);
			}
		}

		if (ver2 >= 2)
		{
			bool bDisableAutoLOD;
			f.serial(bDisableAutoLOD);
			disableAutoLOD(bDisableAutoLOD);
		}

		uint32 tailNbSegs;
		bool   colorFading;
		bool   systemBasisEnabled;
		bool   drEnabled; // dying ribbons, not supported in this version

		CPSColoredParticle::serialColorScheme(f);
		CPSSizedParticle::serialSizeScheme(f);

		// we dont use the 2d angle anymore...serial a dummy one
		{
			struct CDummy2DAngle : CPSRotated2DParticle
			{
				CPSLocated *getAngle2DOwner(void) { return NULL; }
			};

			 CDummy2DAngle _Dummy2DAngle;
			_Dummy2DAngle.serialAngle2DScheme(f); 
		}

		f.serial(colorFading, systemBasisEnabled);
		serialMaterial(f);

		f.serial(drEnabled);				
		f.serial(tailNbSegs);
		ITexture *tex;
		f.serialPolyPtr(tex);
		_Tex = tex;
		if (_Tex != NULL)
		{
			f.serial(_UFactor, _VFactor) ;
		}

		// shape serialization	
		f.serialCont(_Shape);
		
		
		_NbSegs = tailNbSegs >> 1;
		if (_NbSegs < 1) _NbSegs = 2;
		setInterpolationMode(Linear);

		nlassert(_Owner);
		resize(_Owner->getMaxSize());
		initDateVect();		
		resetFromOwner();	
	}	


	if (ver >= 2)
	{
		CPSRibbonBase::serial(f);
		CPSColoredParticle::serialColorScheme(f);
		CPSSizedParticle::serialSizeScheme(f);
		CPSMaterial::serialMaterial(f);
		f.serialCont(_Shape);
		bool colorFading = _ColorFading;
		f.serial(colorFading);	
		_ColorFading = colorFading;
		uint32 tailNbSegs = _NbSegs;
		f.serial(tailNbSegs);		
		if (f.isReading())
		{
			setTailNbSeg(_NbSegs);
			touch();
		}
		ITexture *tex = _Tex;
		f.serialPolyPtr(tex);
		_Tex = tex;
		if (_Tex != NULL)
		{
			f.serial(_UFactor, _VFactor) ;
		}
	}

}


//=======================================================	
CPSRibbon::CPSRibbon() 
  : _ColorFading(true),
	_GlobalColor(false),
	_Touch(true),
	_UFactor(1.f),
	_VFactor(1.f)
{
	setInterpolationMode(Linear);
	setSegDuration(0.06f);
	_Name = std::string("Ribbon");
	setShape(Triangle, NbVerticesInTriangle);
	_Mat.setDoubleSided(true);
}


//=======================================================	
CPSRibbon::~CPSRibbon()
{
}


//==========================================================================	
inline uint CPSRibbon::getNumVerticesInSlice() const
{
	return _Shape.size() + (_Tex == NULL ? 0 : 1);
}





//=======================================================	
void CPSRibbon::step(TPSProcessPass pass, TAnimationTime ellapsedTime, TAnimationTime realEt)
{	
	if (pass == PSMotion)
	{	
		if (!_Parametric)
		{
			updateGlobals();
		}
	}
	else
	if (
		(pass == PSBlendRender && hasTransparentFaces())
		|| (pass == PSSolidRender && hasOpaqueFaces())
		)
	{
		uint32 step;
		uint   numToProcess;
		computeSrcStep(step, numToProcess);	
		if (!numToProcess) return;
		
		/// update the material color
		CParticleSystem &ps = *(_Owner->getOwner());	
		_Mat.setColor(ps.getGlobalColor());
		
		/** We support Auto-LOD for ribbons, although there is a built-in LOD (that change the geometry rather than the number of ribbons)
		  * that gives better result (both can be used simultaneously)
		  */

		displayRibbons(numToProcess, step);

	}
	else 
	if (pass == PSToolRender) // edition mode only
	{			
		//showTool();
	}	
}


//=======================================================	
void CPSRibbon::newElement(CPSLocated *emitterLocated, uint32 emitterIndex)
{
	CPSRibbonBase::newElement(emitterLocated, emitterIndex);
	newColorElement(emitterLocated, emitterIndex);
	newSizeElement(emitterLocated, emitterIndex);
}


//=======================================================	
void CPSRibbon::deleteElement(uint32 index)
{
	CPSRibbonBase::deleteElement(index);
	deleteColorElement(index);	
	deleteSizeElement(index);	
}


//=======================================================	
void CPSRibbon::resize(uint32 size)
{
	nlassert(size < (1 << 16));
	CPSRibbonBase::resize(size);	
	resizeColor(size);
	resizeSize(size);
}

//=======================================================	
void CPSRibbon::updateMatAndVbForColor(void)
{
	touch();
}


///=========================================================================
// Create the start slice of a ribbon (all vertices at the same pos)
static inline uint8 *BuildRibbonFirstSlice(const NLMISC::CVector &pos,
										   uint  numVerts,
										   uint8 *dest,
										   uint  vertexSize
									      )
{	
	do
	{
		* (NLMISC::CVector *) dest = pos;
		dest += vertexSize;
	}
	while (--numVerts);
	return dest;
}


///=========================================================================
// This compute one slice of a ribbon, and return the next vertex to be filled
static inline uint8 *ComputeRibbonSlice(const NLMISC::CVector &prev,
									    const NLMISC::CVector &next,									 
									    const NLMISC::CVector *shape,
									    uint  numVerts,
									    uint8 *dest,
									    uint  vertexSize,
										float size
									   )
{	
	// compute a basis from the next and previous position.
	// (not optimized for now, but not widely used, either...)	
	static NLMISC::CMatrix m;
	m.setPos(next);
	CPSUtil::buildSchmidtBasis(next - prev, m);
	m.scale(size);
	
	const NLMISC::CVector *shapeEnd = shape + numVerts;
	do
	{
		*(NLMISC::CVector *) dest = m * (*shape);
		++shape;
		dest += vertexSize;
	}
	while (shape != shapeEnd);
	return dest;
}



///=========================================================================
// This is used to compute a ribbon mesh from its curve and its base shape.
// This is for untextured versions (no need to duplicate the last vertex of each slice)
static inline uint8 *ComputeUntexturedRibbonMesh(uint8 *destVb,
											     uint  vertexSize,
											     const NLMISC::CVector *curve,
											     const NLMISC::CVector *shape,
											     uint  numSegs,
												 uint  numVerticesInShape,
												 float sizeIncrement,
												 float size
												)
{					
	do
	{
		destVb = ComputeRibbonSlice(curve[1],
								    curve[0],
									shape,
									numVerticesInShape,
									destVb,
									vertexSize,
									size);
		++ curve;
		size -= sizeIncrement;
	}
	while (--numSegs);
	return BuildRibbonFirstSlice(curve[0], numVerticesInShape, destVb, vertexSize);  		
}

///=========================================================================
// This is used to compute a ribbon mesh from its curve and its base shape.
// (Textured Version)
static inline uint8 *ComputeTexturedRibbonMesh(uint8 *destVb,
											   uint  vertexSize,
											   const NLMISC::CVector *curve,
											   const NLMISC::CVector *shape,
											   uint  numSegs,
											   uint  numVerticesInShape,
											   float sizeIncrement,
											   float size
											  )
{	
			
	do
	{
		uint8 *nextDestVb = ComputeRibbonSlice(curve[1],
											   curve[0],
											   shape,
											   numVerticesInShape,
											   destVb,
											   vertexSize,
											   size
											  );
		// duplicate last vertex ( equal first)
		* (NLMISC::CVector *) nextDestVb = * (NLMISC::CVector *) destVb;
		destVb = nextDestVb + vertexSize;
		//
		++ curve;
		size -= sizeIncrement;
	}
	while (--numSegs);
	return BuildRibbonFirstSlice(curve[0], numVerticesInShape + 1, destVb, vertexSize);  	
}

//==========================================================================	
void CPSRibbon::displayRibbons(uint32 nbRibbons, uint32 srcStep)
{	
	if (!nbRibbons) return;
	nlassert(_Owner);	
	CPSRibbonBase::updateLOD();
	if (_UsedNbSegs < 2) return;
	const float date = _Owner->getOwner()->getSystemDate();
	uint8						*currVert;
	CVBnPB						&VBnPB = getVBnPB(); // get the appropriate vb (built it if needed)
	CVertexBuffer				&VB = VBnPB.VB;
	CPrimitiveBlock				&PB = VBnPB.PB;
	const uint32				vertexSize  = VB.getVertexSize();
	uint						colorOffset;	
	
	IDriver *drv = this->getDriver();
	setupDriverModelMatrix();
	drv->activeVertexBuffer(VB);
	_Owner->incrementNbDrawnParticles(nbRibbons); // for benchmark purpose		
	const uint numRibbonBatch = getNumRibbonsInVB(); // number of ribons to process at once		
	if (_UsedNbSegs == 0) return;	
	////////////////////
	// Material setup //
	////////////////////
		CParticleSystem &ps = *(_Owner->getOwner());
		bool useGlobalColor = ps.getColorAttenuationScheme() != NULL;
		if (useGlobalColor != _GlobalColor)
		{
			touch();
		}
		updateMaterial();
		setupGlobalColor();
		//
		if (_ColorScheme)
		{
			colorOffset = VB.getColorOff();	
		}	
	/////////////////////
	// Compute ribbons //
	/////////////////////
		const uint numVerticesInSlice = getNumVerticesInSlice();
		const uint numVerticesInShape = _Shape.size();
		//
		static std::vector<float> sizes;
		static std::vector<NLMISC::CVector> ribbonPos;  // this is where the position of each ribbon slice center i stored
		ribbonPos.resize(_UsedNbSegs + 1); // make sure we have enough room
		sizes.resize(numRibbonBatch);
		
		//
		uint toProcess;
		uint ribbonIndex = 0; // index of the first ribbon in the batch being processed	
		uint32 fpRibbonIndex = 0; // fixed point index in source
		do
		{
			toProcess = std::min((uint) (nbRibbons - ribbonIndex) , numRibbonBatch);
			currVert = (uint8 *) VB.getVertexCoordPointer();
				
			/// setup sizes
			const float	*ptCurrSize;
			uint32  ptCurrSizeIncrement;
			if (_SizeScheme)
			{			
				ptCurrSize = (float *) _SizeScheme->make(this->_Owner, ribbonIndex, &sizes[0], sizeof(float), toProcess, true, srcStep);			
				ptCurrSizeIncrement = 1;
			}
			else
			{
				ptCurrSize = &_ParticleSize;
				ptCurrSizeIncrement = 0;
			}

			/// compute colors
			if (_ColorScheme)
			{			
				_ColorScheme->makeN(this->_Owner, ribbonIndex, currVert + colorOffset, vertexSize, toProcess, numVerticesInSlice * (_UsedNbSegs + 1), srcStep);			
			}			
			uint k = toProcess;	
			//////////////////////////////////////////////////////////////////////////////////////
			// interpolate and project points the result is directly setup in the vertex buffer //
			//////////////////////////////////////////////////////////////////////////////////////
			if (!_Parametric)
			{
				//////////////////////
				// INCREMENTAL CASE //
				//////////////////////				
				if (_Tex != NULL) // textured case
				{
					do
					{
						const float ribbonSizeIncrement = *ptCurrSize / (float) _UsedNbSegs;
						ptCurrSize += ptCurrSizeIncrement;
						// the parent class has a method to get the ribbons positions
						computeRibbon((uint) (fpRibbonIndex >> 16), &ribbonPos[0], sizeof(NLMISC::CVector));
						currVert = ComputeTexturedRibbonMesh(currVert,
															 vertexSize,
															 &ribbonPos[0],
															 &_Shape[0],
															 _UsedNbSegs,
															 numVerticesInShape,
															 ribbonSizeIncrement,
															 *ptCurrSize
															);
						fpRibbonIndex += srcStep;
					}
					while (--k);
				}
				else // untextured case
				{
					do
					{
						const float ribbonSizeIncrement = *ptCurrSize / (float) _UsedNbSegs;
						ptCurrSize += ptCurrSizeIncrement;
						// the parent class has a method to get the ribbons positions
						computeRibbon((uint) (fpRibbonIndex >> 16), &ribbonPos[0], sizeof(NLMISC::CVector));
						currVert = ComputeUntexturedRibbonMesh(currVert,
															   vertexSize,
															   &ribbonPos[0],
															   &_Shape[0],
															   _UsedNbSegs,
															   numVerticesInShape,
															   ribbonSizeIncrement,
															   *ptCurrSize
															  );
						fpRibbonIndex += srcStep;
					}
					while (--k);	
				}
			}
			else
			{
				//////////////////////
				// PARAMETRIC  CASE //
				//////////////////////				
				if (_Tex != NULL) // textured case
				{
					do
					{
						const float ribbonSizeIncrement = *ptCurrSize / (float) _UsedNbSegs;
						ptCurrSize += ptCurrSizeIncrement;
						_Owner->integrateSingle(date - _UsedSegDuration * (_UsedNbSegs + 1),
												_UsedSegDuration,
												_UsedNbSegs + 1,
												(uint) (fpRibbonIndex >> 16),
												&ribbonPos[0]);

						currVert = ComputeTexturedRibbonMesh(currVert,
															 vertexSize,
															 &ribbonPos[0],
															 &_Shape[0],
															 _UsedNbSegs,
															 numVerticesInShape,
															 ribbonSizeIncrement,
															 *ptCurrSize
														    );
						fpRibbonIndex += srcStep;
					}
					while (--k);
				}
				else // untextured case
				{
					do
					{
						const float ribbonSizeIncrement = *ptCurrSize / (float) _UsedNbSegs;
						ptCurrSize += ptCurrSizeIncrement;
						_Owner->integrateSingle(date - _UsedSegDuration * (_UsedNbSegs + 1),
												_UsedSegDuration,
												_UsedNbSegs + 1,
												(uint) (fpRibbonIndex >> 16),
												&ribbonPos[0]);

						currVert = ComputeUntexturedRibbonMesh(currVert,
															   vertexSize,
															   &ribbonPos[0],
															   &_Shape[0],
															   _UsedNbSegs,
															   numVerticesInShape,
															   ribbonSizeIncrement,
															   *ptCurrSize
															  );				
						fpRibbonIndex += srcStep;
					}
					while (--k);
				}							
			}			
			// display the result
			PB.setNumTri((numVerticesInShape * _UsedNbSegs * toProcess) << 1);			
			drv->render(PB, _Mat);
			ribbonIndex += toProcess;		
		}
		while (ribbonIndex != nbRibbons);
		
}	

//==========================================================================	
bool CPSRibbon::hasTransparentFaces(void)
{
	return getBlendingMode() != CPSMaterial::alphaTest ;
}


//==========================================================================	
bool CPSRibbon::hasOpaqueFaces(void)
{
	return !hasTransparentFaces();
}

//==========================================================================	
uint32 CPSRibbon::getMaxNumFaces(void) const
{
	nlassert(_Owner);
	return _Owner->getMaxSize() * _NbSegs;	
}

//==========================================================================	
CPSRibbon::CVBnPB &CPSRibbon::getVBnPB()
{
	static TVBMap * const vbMaps[] =
	{
		/// untextured cases
		&_VBMap,		
		&_FadedVBMap,	     
		&_ColoredVBMap,  
		&_FadedColoredVBMap,
		/// textured cases
		&_TexVBMap,		
		&_TexFadedVBMap,	     
		&_TexColoredVBMap,  
		&_TexFadedColoredVBMap
	};

	/// choose the right vb by building an index for lookup into 'vbMaps' defined above
	TVBMap &map = *vbMaps[ (_Tex != NULL			 ? (1 << 2) : 0)  | // set bit 2 if textured
						   (_ColorScheme != NULL  ? (1 << 1) : 0)  | // set bit 1 if per ribbon color
						   (_ColorFading			 ? 1 : 0)		   // set bit 0 if color fading
						 ];

	const uint numVerticesInSlice = getNumVerticesInSlice(); /// 1 vertex added for textured ribbon (to avoid texture stretching)
	const uint numVerticesInShape = _Shape.size();


	// The number of slice is encoded in the upper word of the vb index
	// The number of vertices per slices is encoded in the lower word
	uint VBnPDIndex = ((_UsedNbSegs + 1) << 16) | numVerticesInSlice;
	TVBMap::iterator it = map.find(VBnPDIndex);
	if (it != map.end())
	{
		return it->second;
	}
	else	// must create this vb, with few different size, it is still interseting, though they are only destroyed at exit
	{
		const uint numRibbonInVB = getNumRibbonsInVB();
		CVBnPB &VBnPB = map[VBnPDIndex]; // make an entry

		/// set the vb format & size
		/// In the case of a ribbon with color and fading, we encode the fading in a texture
		/// If the ribbon has fading, but only a global color, we encode it in the primary color
		CVertexBuffer &vb = VBnPB.VB;
		vb.setVertexFormat(CVertexBuffer::PositionFlag	| /* alway need position */
						   (_ColorScheme || _ColorFading ? CVertexBuffer::PrimaryColorFlag : 0) | /* need a color ? */
						   ((_ColorScheme && _ColorFading) ||  _Tex != NULL ? CVertexBuffer::TexCoord0Flag : 0) | /* need texture coordinates ? */
						   (_Tex != NULL && _ColorScheme && _ColorFading ? CVertexBuffer::TexCoord1Flag : 0) /* need 2nd texture coordinates ? */
						  );
		vb.setNumVertices((_UsedNbSegs + 1) * numRibbonInVB * numVerticesInSlice); // 1 seg = 1 line + terminal vertices

		// set the primitive block size
		CPrimitiveBlock &pb = VBnPB.PB;
		pb.setNumTri((_UsedNbSegs * numRibbonInVB * _Shape.size()) << 1);
		/// Setup the pb and vb parts. Not very fast but executed only once
		uint vbIndex = 0;
		uint pbIndex = 0; 
		uint i, k, l;
		for (i = 0; i < numRibbonInVB; ++i)
		{
			for (k = 0; k < (_UsedNbSegs + 1); ++k)
			{

				/// setup primitive block
				if (k != _UsedNbSegs) /// there are alway one more slice than segments in the ribbon...
				{	
					uint vIndex = vbIndex;
					for (l = 0; l < (numVerticesInShape - 1); ++l) /// deals with each ribbon vertices
					{																			
						pb.setTri(pbIndex ++, vIndex, vIndex + numVerticesInSlice, vIndex + numVerticesInSlice + 1);
						pb.setTri(pbIndex ++, vIndex, vIndex + numVerticesInSlice + 1, vIndex + 1);
						++ vIndex;
					}	
					
					/// the last 2 index don't loop if there's a texture
					uint nextVertexIndex = (numVerticesInShape == numVerticesInSlice) ?	vIndex + 1 - numVerticesInShape // no texture -> we loop 
										   : vIndex + 1; // a texture is used : use onemore vertex										
					pb.setTri(pbIndex ++, vIndex, vIndex + numVerticesInSlice, nextVertexIndex + numVerticesInSlice);
					pb.setTri(pbIndex ++, vIndex, nextVertexIndex + numVerticesInSlice, nextVertexIndex);

				}

				/// setup vb
				for (l = 0; l < numVerticesInSlice; ++l) /// deals with each ribbon vertices
				{
					nlassert(vbIndex < vb.getNumVertices());
					/// setup texture (if any)
					if (_Tex != NULL)
					{						
						vb.setTexCoord(vbIndex,
									   _ColorScheme && _ColorFading ? 1 : 0,		// must we use the second texture coord ? (when 1st one used by the gradient texture : we can't encode it in the diffuse as it encodes each ribbon color)
									   (float) k / _UsedNbSegs,				  // u
									   1.f - (l / (float) numVerticesInShape) // v
									  );						
					}

					/// setup gradient
					if (_ColorFading)
					{
						// If not per ribbon color, we can encode it in the diffuse
						if (_ColorScheme == NULL)
						{
							uint8 intensity = (uint8) (255 * (1.f - ((float) k / _UsedNbSegs)));
							NLMISC::CRGBA col(intensity, intensity, intensity, intensity);
							vb.setColor(vbIndex, col);						
						}
						else // encode it in the first texture
						{
							vb.setTexCoord(vbIndex, 0, 0.5f - 0.5f * ((float) k / _UsedNbSegs), 0);
						}
					}
					++ vbIndex;
				}
				
			}
		}
		return VBnPB;
	}
}

//==========================================================================	
uint	CPSRibbon::getNumRibbonsInVB() const
{
	const uint numVerticesInSlice = getNumVerticesInSlice(); /// 1 vertex added for textured ribbon (to avoid texture stretching)
	const uint vertexInVB = 512;	
	return std::max(1u, (uint) (vertexInVB / (numVerticesInSlice * (_UsedNbSegs + 1))));
}


//==========================================================================	
inline void	CPSRibbon::updateUntexturedMaterial()
{
	///////////////////////
	// UNTEXTURED RIBBON //
	///////////////////////

	static NLMISC::CRefPtr<ITexture> ptGradTexture;

	CParticleSystem &ps = *(_Owner->getOwner());
	if (_ColorScheme)
	{	// PER RIBBON COLOR
		if (ps.getColorAttenuationScheme())
		{
			if (_ColorFading) // global color + fading + per ribbon color
			{
				// the first stage is used to get fading * global color
				// the second stage multiply the result by the diffuse colot
				if (ptGradTexture == NULL) // have we got a gradient texture ?
				{
					ptGradTexture = CreateGradientTexture();
				}
				_Mat.setTexture(0, ptGradTexture);
				CPSMaterial::forceTexturedMaterialStages(2); // use constant color 0 * diffuse, 1 stage needed
				SetupModulatedStage(_Mat, 0, CMaterial::Texture, CMaterial::Constant);
				SetupModulatedStage(_Mat, 1, CMaterial::Previous, CMaterial::Diffuse);
			}
			else // per ribbon color with global color 
			{
				CPSMaterial::forceTexturedMaterialStages(1); // use constant color 0 * diffuse, 1 stage needed
				SetupModulatedStage(_Mat, 0, CMaterial::Diffuse, CMaterial::Constant);
			}
		}
		else
		{	
			if (_ColorFading) // per ribbon color, no fading
			{				
				if (ptGradTexture == NULL) // have we got a gradient texture ?
				{
					ptGradTexture = CreateGradientTexture();
				}
				_Mat.setTexture(0, ptGradTexture);
				CPSMaterial::forceTexturedMaterialStages(1);
				SetupModulatedStage(_Mat, 0, CMaterial::Texture, CMaterial::Diffuse);
			}
			else // per color ribbon with no fading, and no global color
			{
				CPSMaterial::forceTexturedMaterialStages(0); // no texture use constant diffuse only
			}
		}
	}
	else // GLOBAL COLOR
	{		
		if (_ColorFading)
		{								
			CPSMaterial::forceTexturedMaterialStages(1); // use constant color 0 * diffuse, 1 stage needed				
			SetupModulatedStage(_Mat, 0, CMaterial::Diffuse, CMaterial::Constant);
		}
		else // color attenuation, no fading : 
		{
			CPSMaterial::forceTexturedMaterialStages(0); // no texture use constant diffuse only				
		}		
	}
	_Touch = false;
}

//==========================================================================	
inline void	CPSRibbon::updateTexturedMaterial()
{
	/////////////////////
	// TEXTURED RIBBON //
	/////////////////////

	static NLMISC::CRefPtr<ITexture> ptGradTexture;
	CParticleSystem &ps = *(_Owner->getOwner());
	if (_ColorScheme)
	{	// PER RIBBON COLOR
		if (ps.getColorAttenuationScheme())
		{
			if (_ColorFading) // global color + fading + per ribbon color
			{				
				if (ptGradTexture == NULL) // have we got a gradient texture ?
				{
					ptGradTexture = CreateGradientTexture(); // create it
				}
				/// fading is stored in last stage (work only with 3 stages...)
				_Mat.setTexture(0, ptGradTexture);
				_Mat.setTexture(1, _Tex);
				CPSMaterial::forceTexturedMaterialStages(3); // use constant color 0 * diffuse, 1 stage needed				
				SetupModulatedStage(_Mat, 0, CMaterial::Texture, CMaterial::Diffuse);
				SetupModulatedStage(_Mat, 1, CMaterial::Texture, CMaterial::Previous);
				SetupModulatedStage(_Mat, 2, CMaterial::Previous, CMaterial::Constant);				
			}
			else // per ribbon color with global color 
			{
				_Mat.setTexture(0, _Tex);
				CPSMaterial::forceTexturedMaterialStages(2); // use constant color 0 * diffuse, 1 stage needed				
				SetupModulatedStage(_Mat, 0, CMaterial::Texture, CMaterial::Diffuse);
				SetupModulatedStage(_Mat, 1, CMaterial::Previous, CMaterial::Constant);			
			}
		}
		else
		{	
			if (_ColorFading) // per ribbon color, fading : 2 textures needed
			{				
				if (ptGradTexture == NULL) // have we got a gradient texture ?
				{
					ptGradTexture = CreateGradientTexture(); // create it
				}
				_Mat.setTexture(0, ptGradTexture);
				_Mat.setTexture(1, _Tex);
				CPSMaterial::forceTexturedMaterialStages(2); 
				SetupModulatedStage(_Mat, 0, CMaterial::Texture, CMaterial::Diffuse); // texture * ribbon color
				SetupModulatedStage(_Mat, 1, CMaterial::Texture, CMaterial::Previous);	// * gradient
			}
			else // per color ribbon with no fading, and no global color
			{
				_Mat.setTexture(0, _Tex);
				CPSMaterial::forceTexturedMaterialStages(1); // no texture use constant diffuse only
				SetupModulatedStage(_Mat, 0, CMaterial::Texture, CMaterial::Diffuse);				
			}
		}
	}
	else // GLOBAL COLOR
	{
		
		if (_ColorFading) // gradient is encoded in diffuse
		{
			_Mat.setTexture(0, _Tex);
			CPSMaterial::forceTexturedMaterialStages(2); // use constant color 0 * diffuse, 1 stage needed				
			SetupModulatedStage(_Mat, 0, CMaterial::Texture, CMaterial::Diffuse);
			SetupModulatedStage(_Mat, 1, CMaterial::Previous, CMaterial::Constant);			
		}
		else // constant color
		{
			_Mat.setTexture(0, _Tex);
			CPSMaterial::forceTexturedMaterialStages(1); // no texture use constant diffuse only							
			SetupModulatedStage(_Mat, 0, CMaterial::Texture, CMaterial::Diffuse);	
		}		
	}
	_Touch = false;
}

//==========================================================================	
void	CPSRibbon::updateMaterial()
{
	if (!_Touch) return;
	if (_Tex != NULL) 
	{
		updateTexturedMaterial();
		setupTextureMatrix();
	}
	else
	{
		updateUntexturedMaterial();
	}
}



//==========================================================================	
inline void	CPSRibbon::setupUntexturedGlobalColor()
{	
	/// setup the global color if it is used
	CParticleSystem &ps = *(_Owner->getOwner());	
	if (_ColorScheme)
	{	
		_Mat.texConstantColor(0, ps.getGlobalColor());					
	}
	else // GLOBAL COLOR with / without fading
	{
		if (ps.getColorAttenuationScheme())
		{			
			NLMISC::CRGBA col;
			col.modulateFromColor(ps.getGlobalColor(), _Color);
			if (_ColorFading)
			{								
				_Mat.texConstantColor(0, col);				
			}
			else // color attenuation, no fading : 
			{							
				_Mat.setColor(col);
			}
		}
		else
		{
			if (_ColorFading)
			{
				_Mat.texConstantColor(0, _Color);				
			}
			else // constant color
			{
				_Mat.setColor(_Color);
			}
		}
	}
}

//==========================================================================	
inline void	CPSRibbon::setupTexturedGlobalColor()
{	
	/// setup the global color if it is used
	CParticleSystem &ps = *(_Owner->getOwner());	
	if (_ColorScheme)
	{	
		if (ps.getColorAttenuationScheme() && _ColorFading)
		{
			_Mat.texConstantColor(2, ps.getGlobalColor());
		}
		else
		{
			_Mat.texConstantColor(1, ps.getGlobalColor());
		}				
	}
	else // GLOBAL COLOR with / without fading
	{
		if (ps.getColorAttenuationScheme())
		{			
			NLMISC::CRGBA col;
			col.modulateFromColor(ps.getGlobalColor(), _Color);
			if (_ColorFading)
			{								
				_Mat.texConstantColor(1, col);				
			}
			else // color attenuation, no fading : 
			{							
				_Mat.setColor(col);
			}
		}
		else
		{
			if (_ColorFading)
			{
				_Mat.texConstantColor(1, _Color);				
			}
			else // constant color
			{
				_Mat.setColor(_Color);
			}
		}
	}
}


//==========================================================================	
void	CPSRibbon::setupGlobalColor()
{
	if (_Tex != NULL) setupTexturedGlobalColor();
		else setupUntexturedGlobalColor();
}

//==========================================================================	
void CPSRibbon::setupTextureMatrix()
{
	uint stage = (_ColorScheme != NULL && _ColorFading == true) ? 1 : 0;	
	if (_UFactor != 1.f || _VFactor != 1.f)
	{
		_Mat.enableUserTexMat(stage);
		CMatrix texMat;
		texMat.setRot(_UFactor  * NLMISC::CVector::I,
					  _VFactor  * NLMISC::CVector::J,
					  NLMISC::CVector::K
					 );
		_Mat.setUserTexMat(stage, texMat);
	}
	else
	{
		_Mat.enableUserTexMat(stage, false);
	}
	_Mat.enableUserTexMat(1 - stage, false);
}

//==========================================================================	
///==================================================================================================================
void CPSRibbon::setShape(const CVector *shape, uint32 nbPointsInShape)
{
	nlassert(nbPointsInShape >= 3);		
	_Shape.resize(nbPointsInShape);
	std::copy(shape, shape + nbPointsInShape, _Shape.begin());	
}

///==================================================================================================================
void CPSRibbon::getShape(CVector *shape) const
{
	std::copy(_Shape.begin(), _Shape.end(), shape);
}



} // NL3D
