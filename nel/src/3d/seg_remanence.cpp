/** \file seg_remanence.cpp
 *
 * $Id: seg_remanence.cpp,v 1.17 2004/06/24 17:33:08 berenguier Exp $
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

#include "3d/scene_group.h"
#include "3d/seg_remanence.h"
#include "3d/seg_remanence_shape.h"
#include "3d/vertex_buffer.h"
#include "3d/driver.h"
#include "3d/scene.h"
#include "3d/anim_detail_trav.h"
#include "3d/skeleton_model.h"
#include "3d/dru.h"






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
CSegRemanence::CSegRemanence() : _NumSlice(0),
								 _Started(false),
								 _Stopping(false),
								 _Restarted(false),
								 _UnrollRatio(0),
								 _SliceTime(0.05f),
								 _AniMat(NULL),
								 _LastSampleFrame(0)
{	
	IAnimatable::resize(AnimValueLast);

	// RenderFilter: We are a SegRemanece
	_RenderFilterType= UScene::FilterSegRemanence;
}

//===============================================================
CSegRemanence::~CSegRemanence()
{
	delete _AniMat;
	// Auto detach me from skeleton. Must do it here, not in ~CTransform().
	if(_FatherSkeletonModel)
	{
		// detach me from the skeleton.
		// hrc and clip hierarchy is modified.
		_FatherSkeletonModel->detachSkeletonSon(this);
		nlassert(_FatherSkeletonModel==NULL);
	}
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
	_Stopping   = other._Stopping;
	_StartDate  = other._StartDate;
	_CurrDate   = other._CurrDate;
	
}

//===============================================================
void CSegRemanence::registerBasic()
{
	CScene::registerModel(SegRemanenceShapeId, TransformShapeId, CSegRemanence::creator);	
}



//===============================================================
void CSegRemanence::render(IDriver *drv, CVertexBuffer &vb, CIndexBuffer &pb, CMaterial &mat)
{
	CSegRemanenceShape *srs = NLMISC::safe_cast<CSegRemanenceShape *>((IShape *) Shape);		
	const uint vertexSize = vb.getVertexSize();
	{
		CVertexBufferReadWrite vba;
		vb.lock (vba);
		uint8 *datas = (uint8 *) vba.getVertexCoordPointer();
		uint numCorners = _Ribbons.size();
		uint k;		
		for(k = 0; k < numCorners; ++k)
		{
			_Ribbons[k].fillVB(datas, vertexSize, srs->getNumSlices(), _SliceTime);
			datas += (_NumSlice + 1) * vertexSize;
		}
//#define DEBUG_SEG_REMANENCE_DISPLAY 
#ifdef DEBUG_SEG_REMANENCE_DISPLAY		
		drv->setupModelMatrix(CMatrix::Identity);
		if (!numCorners) return;
		/*
		for(k = 0; k < numCorners - 1; ++k)
		{
			_Ribbons[k].fillVB(datas, vertexSize, srs->getNumSlices(), _SliceTime);
			datas += (_NumSlice + 1) * vertexSize;
			CDRU::drawLine(srs->getCorner(k), srs->getCorner(k + 1), CRGBA::White, *drv);
		}
		*/
		CVertexBufferReadWrite bfrw;
		vb.lock(bfrw);
		datas = (uint8 *) bfrw.getVertexCoordPointer();
		for(uint k = 0; k < _NumSlice - 1; ++k)
		{
			for(uint l = 0; l < _NumCorners - 1; ++l)
			{
				const NLMISC::CVector &v0 = *(const NLMISC::CVector *) (datas + vertexSize * (k + l * (_NumSlice + 1)));
				const NLMISC::CVector &v1 = *(const NLMISC::CVector *) (datas + vertexSize * (k + 1 + l * (_NumSlice + 1)));
				const NLMISC::CVector &v2 = *(const NLMISC::CVector *) (datas + vertexSize * (k + 1 + (l + 1) * (_NumSlice + 1)));
				const NLMISC::CVector &v3 = *(const NLMISC::CVector *) (datas + vertexSize * (k + 1 + (l + 1) * (_NumSlice + 1)));
				CDRU::drawLine(v0, v1, CRGBA::White, *drv);
				CDRU::drawLine(v0, v3, CRGBA::White, *drv);
				CDRU::drawLine(v0, v2, CRGBA::White, *drv);
			}
		}		
#endif
	}
	
	// roll / unroll using texture matrix
	CMatrix texMat;	
	texMat.setPos(NLMISC::CVector(1.f - _UnrollRatio, 0, 0));
	
	if (mat.getTexture(0) != NULL)
		mat.setUserTexMat(0, texMat);		
	drv->setupModelMatrix(CMatrix::Identity);
	
	drv->activeVertexBuffer(vb);	
	drv->activeIndexBuffer(pb);
	drv->renderTriangles(mat, 0, pb.getNumIndexes()/3);

	CScene *scene = getOwnerScene();
	// change unroll ratio
	if (!_Stopping)
	{
		if (_UnrollRatio != 1.f)
		_UnrollRatio = std::min(1.f, _UnrollRatio + scene->getEllapsedTime() / (srs->getNumSlices() * _SliceTime));
	}
	else
	{
		_UnrollRatio = std::max(0.f, _UnrollRatio - srs->getRollupRatio() * scene->getEllapsedTime() / (srs->getNumSlices() * _SliceTime));
		if (_UnrollRatio == 0.f)
		{
			_Stopping = false;
			_Started = false;
		}
	}	
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
		CSegRemanenceShape *srs = NLMISC::safe_cast<CSegRemanenceShape *>((IShape *) Shape);
		uint numCorners = _Ribbons.size();
		for(uint k = 0; k < numCorners; ++k)
		{		
			_Ribbons[k].samplePos(getWorldMatrix() * srs->getCorner(k), date, _SliceTime);
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
/*
void CSegRemanence::traverseHrc()
{
	CTransformShape::traverseHrc ();
	if (isStarted())
	{	
		CScene *scene = getOwnerScene();
		if (scene->getNumRender() != (_LastSampleFrame + 1))
		{
			if (!isStopping())
			{			
				// if wasn't visible at previous frame, must invalidate position
				restart();
			}
			else
			{
				// ribbon started unrolling when it disapperaed from screen so simply remove it
				stopNoUnroll();
			}
		}		
		_LastSampleFrame = scene->getNumRender();
		setupFromShape();
		samplePos((float) scene->getCurrentTime());
	}
}
*/

//===============================================================
void CSegRemanence::start()
{
	if (_SliceTime == 0.f) return;
	if (_Started && !_Stopping) return;
	restart();		
}

//===============================================================
void CSegRemanence::restart()
{
	CSegRemanenceShape *srs = NLMISC::safe_cast<CSegRemanenceShape *>((IShape *) Shape);	
	if (!srs->getTextureShifting())
	{	
		_UnrollRatio = 1.f;
	}
	else
	{
		if (!_Stopping)
			_UnrollRatio = 0.f;
	}
	_Started = _Restarted = true;
	_Stopping = false;	
}

//===============================================================
void CSegRemanence::stop()
{
	_Stopping = true;
}

//===============================================================
void CSegRemanence::stopNoUnroll()
{
	_Started = _Restarted = _Stopping = false;
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
ITrack *CSegRemanence::getDefaultTrack (uint valueId)
{	
	CSegRemanenceShape *srs = NLMISC::safe_cast<CSegRemanenceShape *>((IShape *) Shape);
	switch (valueId)
	{
		case PosValue:			return srs->getDefaultPos();		
		case RotQuatValue:		return srs->getDefaultRotQuat();
		case ScaleValue:		return srs->getDefaultScale();		
	}	
	return CTransformShape::getDefaultTrack(valueId);	
	return NULL;

}

//===============================================================
void CSegRemanence::traverseAnimDetail()
{
	CTransformShape::traverseAnimDetail();
	#ifndef DEBUG_SEG_REMANENCE_DISPLAY
		if (isStarted())
	#endif
	{	
		/////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////

		CScene *scene = getOwnerScene();
		if (scene->getNumRender() != (_LastSampleFrame + 1))
		{
			if (!isStopping())
			{			
				// if wasn't visible at previous frame, must invalidate position
				restart();
			}
			else
			{
				// ribbon started unrolling when it disapperaed from screen so simply remove it
				stopNoUnroll();
			}
		}		
		_LastSampleFrame = scene->getNumRender();
		setupFromShape();
		samplePos((float) scene->getCurrentTime());

		/////////////////////////////////////////////////////////////////////////////
		/////////////////////////////////////////////////////////////////////////////

	
		// test if animated material must be updated.
		if(IAnimatable::isTouched(CSegRemanence::OwnerBit))
		{
			if (getAnimatedMaterial())
				getAnimatedMaterial()->update();			
			clearAnimatedMatFlag();
		}
	}
}

//===============================================================
void CSegRemanence::setSliceTime(float duration)
{
	if ( duration != _SliceTime )
	{
		stopNoUnroll();
		_SliceTime = duration;
	}
}

}
