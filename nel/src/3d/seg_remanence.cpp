/** \file seg_remanence.cpp
 *
 * $Id: seg_remanence.cpp,v 1.3 2002/07/03 17:26:56 vizerie Exp $
 */

/* Copyright, 2000, 2001, 2002 Nevrax Ltd.
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

#include "3d/seg_remanence.h"
#include "3d/seg_remanence_shape.h"
#include "3d/vertex_buffer.h"
#include "3d/driver.h"
#include "3d/scene.h"
#include "3d/anim_detail_trav.h"




namespace NL3D
{

/// TODO : put this in a header (same code in ps_ribbon_base.cpp ..)
static inline void BuildHermiteVector(const NLMISC::CVector &P0,
							   const NLMISC::CVector &P1,
							   const NLMISC::CVector &T0,
							   const NLMISC::CVector &T1,
									 NLMISC::CVector &dest,									 
							   float lambda
							   )
{	
	const float lambda2 = lambda * lambda;
	const float lambda3 = lambda2 * lambda;
	const float h1 = 2 * lambda3 - 3 * lambda2 + 1; 
	const float h2 = - 2 * lambda3 + 3 * lambda2; 
	const float h3 = lambda3 - 2 * lambda2 + lambda; 
	const float h4 = lambda3 - lambda2;
	/// just avoid some ctor calls here...
	dest.set (h1 * P0.x + h2 * P1.x + h3 * T0.x + h4 * T1.x,
			  h1 * P0.y + h2 * P1.y + h3 * T0.y + h4 * T1.y,
			  h1 * P0.z + h2 * P1.z + h3 * T0.z + h4 * T1.z);

}

//===============================================================
CSegRemanence::CSegRemanence() : _NumSlice(0), _Started(false), _Restarted(false), _AniMat(NULL)
{	
	IAnimatable::resize(AnimValueLast);
}

//===============================================================
CSegRemanence::~CSegRemanence()
{
	delete _AniMat;
}

//===============================================================
CSegRemanence::CSegRemanence(CSegRemanence &other)	: CTransformShape(other), _AniMat(NULL)
{
	copyFromOther(other);
}

//===============================================================
CSegRemanence &CSegRemanence::operator = (CSegRemanence &other)
{
	if (this != &other)
	{	
		(CTransformShape &) *this = (CTransformShape &) other; // copy base
		copyFromOther(other);
	}
	return *this;
}

//===============================================================
void CSegRemanence::copyFromOther(CSegRemanence &other)
{
	if (this == &other) return;
	
	CAnimatedMaterial   *otherMat = other._AniMat != NULL ? new CAnimatedMaterial(*other._AniMat)
														  : NULL;
	delete _AniMat;
	_AniMat = otherMat;

	_Ribbons	= other._Ribbons; // sampled positions at each extremities of segment
	_NumSlice	= other._NumSlice;
	_NumCorners = other._NumCorners;
	_Started    = other._Started;
	_Restarted  = other._Restarted;
	_StartDate  = other._StartDate;
	_CurrDate   = other._CurrDate;
	
}

//===============================================================
void CSegRemanence::registerBasic()
{
	CMOT::registerModel(SegRemanenceShapeId, TransformShapeId, CSegRemanence::creator);	
	CMOT::registerObs (HrcTravId, SegRemanenceShapeId, CSegRemanenceHrcObs::creator);
	CMOT::registerObs(AnimDetailTravId, SegRemanenceShapeId, CSegRemanenceAnimDetailObs::creator);
}


//===============================================================
void CSegRemanence::render(IDriver *drv, CVertexBuffer &vb, CPrimitiveBlock &pb, CMaterial &mat)
{
	CSegRemanenceShape *srs = NLMISC::safe_cast<CSegRemanenceShape *>((IShape *) Shape);		
	const uint vertexSize = vb.getVertexSize();
	uint8 *datas = (uint8 *) vb.getVertexCoordPointer();
	uint numCorners = _Ribbons.size();
	for(uint k = 0; k < numCorners; ++k)
	{
		_Ribbons[k].fillVB(datas, vertexSize, srs->getNumSlices(), srs->getSliceTime());
		datas += (_NumSlice + 1) * vertexSize;
	}	
	if (srs->getTextureShifting())
	{	
		CMatrix texMat;
		texMat.setPos(NLMISC::CVector(std::max( 1.f - (_CurrDate - _StartDate) / (srs->getNumSlices() * srs->getSliceTime()), 0.f), 0, 0));		
		if (mat.getTexture(0) != NULL)
			mat.setUserTexMat(0, texMat);		
	}
	
	
	drv->activeVertexBuffer(vb);	
	drv->render(pb, mat);

/*	
	drv->setPolygonMode(IDriver::Line);
	CMaterial testMat;
	testMat.texEnvArg0RGB(0, CMaterial::Constant, CMaterial::SrcColor);
	testMat.texEnvOpRGB(0, CMaterial::Replace);
	testMat.setLighting(false);
	testMat.setDoubleSided(true);
	testMat.texConstantColor(0, NLMISC::CRGBA::White);
	drv->render(pb, testMat);
	drv->setPolygonMode(IDriver::Filled);
*/
}

//===============================================================
CSegRemanence::CRibbon::CRibbon() : _LastSamplingDate(0)
{
}

//===============================================================
void CSegRemanence::CRibbon::fillVB(uint8 *dest, uint stride, uint nbSegs, float sliceTime)
{		
	TSampledPosVect::iterator currIt      = _Ribbon.begin();			

	NLMISC::CVector t0 = (currIt + 1)->Pos - currIt->Pos;
	NLMISC::CVector t1 = 0.5f * ((currIt + 2)->Pos - currIt->Pos);

	uint leftToDo = nbSegs + 1;

	float lambda = 0.f;
	float lambdaStep = 1.f;

/*	nlinfo("===============================");
	for(uint k = 0; k < _Ribbon.size(); ++k)
	{
		nlinfo("pos = (%.2f, %.2f, %.2f)", _Ribbon[k].Pos.x, _Ribbon[k].Pos.y, _Ribbon[k].Pos.z);
	}*/

	for (;;)
	{		
		float dt = currIt->SamplingDate - (currIt + 1)->SamplingDate;

		if (dt < 10E-6f) // we reached the start of ribbon
		{

			do
			{
				(NLMISC::CVector &) *dest = currIt->Pos;
				dest  += stride;
			}
			while (--leftToDo);			
			return;
		}

		float newLambdaStep = sliceTime / dt;
		// readapt lambda
		lambda *= newLambdaStep / lambdaStep;
		lambdaStep = newLambdaStep;
		for(;;)
		{
			if (lambda >= 1.f) break;
			/// compute a location
			BuildHermiteVector(currIt->Pos, (currIt + 1)->Pos, t0, t1, (NLMISC::CVector &) *dest, lambda);
			dest  += stride;
			-- leftToDo;
			if (!leftToDo) return;												
			lambda += lambdaStep;			
		}
		
		lambda -= 1.f;

		// Start new segment and compute new tangents
		t0 = t1;
		++currIt;		
		t1 = 0.5f * ((currIt + 2)->Pos - currIt->Pos);
	}	 
}

//===============================================================
void CSegRemanence::CRibbon::duplicateFirstPos()
{
	uint ribSize = _Ribbon.size();
	for(uint k = 1; k < ribSize; ++k)
	{
		_Ribbon[k] = _Ribbon[0];
	}
}

//===============================================================
void CSegRemanence::setupFromShape()
{
	CSegRemanenceShape *srs = NLMISC::safe_cast<CSegRemanenceShape *>((IShape *) Shape);		
	if (srs->getNumCorners() != _NumCorners || srs->getNumSlices() != _NumSlice)
	{
		_Ribbons.resize(srs->getNumCorners());
		for(uint k = 0; k < _Ribbons.size(); ++k)
		{
			_Ribbons[k].setNumSlices(srs->getNumSlices());
		}		
		_NumCorners = srs->getNumCorners();
		_NumSlice  = srs->getNumSlices();
	}
	updateOpacityFromShape();
}


//===============================================================
void CSegRemanence::CRibbon::setNumSlices(uint numSegs)
{	
	_Ribbon.resize(numSegs + 3);
}

//===============================================================
void CSegRemanence::CRibbon::samplePos(const NLMISC::CVector &pos, float date, float sliceDuration)
{
	nlassert(_Ribbon.size() != 0);
	if (date - _LastSamplingDate > sliceDuration)
	{
		_Ribbon.pop_back();
		CSampledPos sp(pos, date);
		_Ribbon.push_front(sp);
		_LastSamplingDate = date;
	}
	else
	{
		_Ribbon.front().Pos = pos;
		_Ribbon.front().SamplingDate = date;
	}
}

//===============================================================
void CSegRemanence::samplePos(float date)
{
	if (_Started)
	{	
		IBaseHrcObs *bho = (IBaseHrcObs *) getObs(HrcTravId);
		CSegRemanenceShape *srs = NLMISC::safe_cast<CSegRemanenceShape *>((IShape *) Shape);
		uint numCorners = _Ribbons.size();
		for(uint k = 0; k < numCorners; ++k)
		{		
			_Ribbons[k].samplePos(bho->WorldMatrix * srs->getCorner(k), date, srs->getSliceTime());
		}
		if (_Restarted)
		{
			for(uint k = 0; k < numCorners; ++k)
			{
				_Ribbons[k].duplicateFirstPos();
			}
			_Restarted = false;
			_StartDate = date;
		}
		_CurrDate = date;
	}
}

//===============================================================
void CSegRemanenceHrcObs::traverse(IObs *caller)
{
	CTransformHrcObs::traverse (caller);
	CSegRemanence *sr = static_cast<CSegRemanence *>(this->Model);
	if (sr->isStarted())
	{	
		CScene *scene = NLMISC::safe_cast<ITravScene *>(Trav)->Scene;
		if (scene->getNumRender() != (_LastSampleFrame + 1))
		{
			// if wasn't visible at previous frame, must invalidate position
			sr->restart();
		}
		_LastSampleFrame = scene->getNumRender();
		sr->setupFromShape();
		sr->samplePos((float) scene->getCurrentTime());
	}
}


//===============================================================
CSegRemanenceHrcObs::CSegRemanenceHrcObs() : _LastSampleFrame(0)
{	
}

//===============================================================
void CSegRemanence::start()
{
	if (_Started) return;
	_Started = true;
	_Restarted = true;
}

//===============================================================
void CSegRemanence::restart()
{
	_Started = _Restarted = true;
}

//===============================================================
void CSegRemanence::stop()
{
	_Restarted = _Started = false;
}

//===============================================================
void CSegRemanence::updateOpacityFromShape()
{
	CSegRemanenceShape *srs = NLMISC::safe_cast<CSegRemanenceShape *>((IShape *) Shape);
	bool transparent = srs->getMaterial().getBlend();
	setTransparency(transparent);
	setOpacity(!transparent);
}

//===============================================================
void CSegRemanence::setAnimatedMaterial(CAnimatedMaterial *mat)
{
	if (mat == _AniMat) return;
	delete _AniMat;
	_AniMat = mat;
	_AniMat->setFather(this, OwnerBit);
}

//===============================================================
void CSegRemanence::registerToChannelMixer(CChannelMixer *chanMixer, const std::string &prefix)
{	
	CTransformShape::registerToChannelMixer(chanMixer, prefix);
	if (_AniMat)
	{
		_AniMat->registerToChannelMixer(chanMixer, prefix + _AniMat->getMaterialName() + ".")	;
	}
}

//===============================================================
void CSegRemanenceAnimDetailObs::traverse(IObs *caller)
{
	CTransformAnimDetailObs::traverse(caller);
	CSegRemanence *sr = NLMISC::safe_cast<CSegRemanence *>(Model);
	
	if (sr->isStarted())
	{	
		// test if animated material must be updated.
		if(sr->IAnimatable::isTouched(CSegRemanence::OwnerBit))
		{
			if (sr->getAnimatedMaterial())
				sr->getAnimatedMaterial()->update();			
			sr->clearAnimatedMatFlag();
		}
	}
}


}
