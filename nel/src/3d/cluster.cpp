/** \file cluster.cpp
 * Implementation of a cluster
 *
 * $Id: cluster.cpp,v 1.7 2002/02/28 12:59:49 besson Exp $
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

#include "3d/cluster.h"
#include "3d/portal.h"
#include "nel/misc/stream.h"
#include "3d/scene.h"
#include "3d/transform_shape.h"
#include "3d/mesh_instance.h"
#include "3d/scene_group.h"

using namespace NLMISC;
using namespace std;

namespace NL3D 
{

// 0.5 cm of precision
#define CLUSTERPRECISION 0.005

// ***************************************************************************
CCluster::CCluster ()
{
	_Obs = NULL;
	FatherVisible = VisibleFromFather = false;
	Father = NULL;
}

// ***************************************************************************
void CCluster::registerBasic ()
{
	CMOT::registerModel (ClusterId, 0, CCluster::creator);
	CMOT::registerObs (ClipTravId, ClusterId, CClusterClipObs::creator);
	CMOT::registerObs (HrcTravId, ClusterId, CClusterHrcObs::creator);
}

// ***************************************************************************
bool CCluster::makeVolume (CVector& p1, CVector& p2, CVector& p3)
{
	uint i;
	// Check if the plane is not close to a plane that already define the cluster
	for (i = 0; i < _LocalVolume.size(); ++i)
	{
		float f1 = fabsf (_LocalVolume[i]*p1);
		float f2 = fabsf (_LocalVolume[i]*p2);
		float f3 = fabsf (_LocalVolume[i]*p3);
		if ((f1 < CLUSTERPRECISION) && (f2 < CLUSTERPRECISION) && (f3 < CLUSTERPRECISION))
			return true;
	}
	// Check if we want to add a triangle not completely in the predefined volume
	for (i = 0; i < _LocalVolume.size(); ++i)
	{
		float f1 = _LocalVolume[i]*p1;
		float f2 = _LocalVolume[i]*p2;
		float f3 = _LocalVolume[i]*p3;
		if ((f1 > CLUSTERPRECISION) && (f2 > CLUSTERPRECISION) && (f3 > CLUSTERPRECISION))
			return false;
	}
	// Build volume
	CPlane p;
	p.make (p1, p2, p3);
	p.normalize();
	_LocalVolume.push_back (p);
	// Build BBox	
	if (_LocalVolume.size() == 1)
		_LocalBBox.setCenter(p1);
	else
		_LocalBBox.extend(p1);
	_LocalBBox.extend(p2);
	_LocalBBox.extend(p3);
	_Volume = _LocalVolume;
	_BBox = _LocalBBox;
	return true;
}

// ***************************************************************************
bool CCluster::isIn (CVector& p)
{
	for (uint i = 0; i < _Volume.size(); ++i)
		if (_Volume[i]*p > CLUSTERPRECISION)
			return false;
	return true;
}


// ***************************************************************************
bool CCluster::isIn (CAABBox& b)
{
	for (uint i = 0; i < _Volume.size(); ++i)
	{
		if (!b.clipBack (_Volume[i]))
			return false;
	}
	return true;
}

// ***************************************************************************
bool CCluster::isIn (NLMISC::CVector& center, float size)
{
	for (uint i = 0; i < _Volume.size(); ++i)
		if (_Volume[i]*center > size)
			return false;
	return true;
}

// ***************************************************************************
void CCluster::resetPortalLinks ()
{
	_Portals.clear();
}

// ***************************************************************************
void CCluster::link (CPortal* portal)
{
	_Portals.push_back (portal);
}

// ***************************************************************************
void CCluster::unlink (CPortal* portal)
{
	uint32 pos;
	for (pos = 0; pos < _Portals.size(); ++pos)
	{
		if (_Portals[pos] == portal)
			break;
	}
	if (pos < _Portals.size())
		_Portals.erase (_Portals.begin()+pos);
}

// ***************************************************************************
void CCluster::serial (IStream&f)
{
	sint version = f.serialVersion (1);

	if (version >= 1)
		f.serial (Name);
	
	f.serialCont (_LocalVolume);
	f.serial (_LocalBBox);
	f.serial (FatherVisible);
	f.serial (VisibleFromFather);
	if (f.isReading())
	{
		_Volume = _LocalVolume;
		_BBox = _LocalBBox;
	}
}

// ***************************************************************************
void CCluster::setWorldMatrix (const CMatrix &WM)
{
	uint32 i;
	CMatrix invWM = WM;
	invWM.invert();

	// Transform the volume
	for (i = 0; i < _LocalVolume.size(); ++i)
		_Volume[i] = _LocalVolume[i] * invWM;

	// Transform the bounding box
	CVector p[8];
	p[0].x = _LocalBBox.getMin().x;
	p[0].y = _LocalBBox.getMin().y;
	p[0].z = _LocalBBox.getMin().z;

	p[1].x = _LocalBBox.getMax().x;
	p[1].y = _LocalBBox.getMin().y;
	p[1].z = _LocalBBox.getMin().z;

	p[2].x = _LocalBBox.getMin().x;
	p[2].y = _LocalBBox.getMax().y;
	p[2].z = _LocalBBox.getMin().z;

	p[3].x = _LocalBBox.getMax().x;
	p[3].y = _LocalBBox.getMax().y;
	p[3].z = _LocalBBox.getMin().z;

	p[4].x = _LocalBBox.getMin().x;
	p[4].y = _LocalBBox.getMin().y;
	p[4].z = _LocalBBox.getMax().z;

	p[5].x = _LocalBBox.getMax().x;
	p[5].y = _LocalBBox.getMin().y;
	p[5].z = _LocalBBox.getMax().z;

	p[6].x = _LocalBBox.getMin().x;
	p[6].y = _LocalBBox.getMax().y;
	p[6].z = _LocalBBox.getMax().z;

	p[7].x = _LocalBBox.getMax().x;
	p[7].y = _LocalBBox.getMax().y;
	p[7].z = _LocalBBox.getMax().z;

	for (i = 0; i < 8; ++i)
		p[i] = WM.mulPoint(p[i]);

	CAABBox boxTemp;

	boxTemp.setCenter(p[0]);
	for (i = 1; i < 8; ++i)
		boxTemp.extend(p[i]);
	_BBox = boxTemp;
}

// ***************************************************************************
CClusterClipObs *CCluster::getClipObs()
{
	if (_Obs == NULL)
		_Obs = (CClusterClipObs*)getObs (ClipTravId);
	return _Obs;
}

// ---------------------------------------------------------------------------
// Observer HRC
// ---------------------------------------------------------------------------

// ***************************************************************************
void CClusterHrcObs::traverse (IObs *caller)
{
	CTransformHrcObs::traverse (caller);

	CCluster *pCluster = static_cast<CCluster*>(this->Model);

	pCluster->setWorldMatrix (WorldMatrix);

	for (uint32 i = 0; i < pCluster->getNbPortals(); ++i)
	{
		CPortal *pPortal = pCluster->getPortal(i);
		pPortal->setWorldMatrix (WorldMatrix);
	}

	// Re affect the cluster to the accelerator if not the root
	if (!pCluster->isRoot())
	{
		pCluster->Group->_ClipTrav->Accel.erase (pCluster->AccelIt);
		pCluster->Group->_ClipTrav->registerCluster (pCluster);
	}
}

// ---------------------------------------------------------------------------
// Observer CLIP
// ---------------------------------------------------------------------------

// ***************************************************************************
CClusterClipObs::CClusterClipObs ()
{
	Visited = false;
}

// ***************************************************************************
bool CClusterClipObs::isRenderable () const
{
	return false;
}

// ***************************************************************************
bool CClusterClipObs::clip (IBaseClipObs *caller)
{
	return true;
}

// ***************************************************************************
void CClusterClipObs::traverse (IObs *caller)
{
	nlassert(!caller || dynamic_cast<IBaseClipObs*>(caller));

	if (Visited)
		return;
	Visited = true;
	// The cluster is visible because we are in it
	// So clip the models attached (with MOT links) to the cluster
	traverseSons();

	// And look through portals
	CCluster *pCluster = static_cast<CCluster*>(this->Model);
	uint32 i;
	for (i = 0; i < pCluster->getNbPortals(); ++i)
	{
		CPortal*pPortal = pCluster->getPortal (i);
		vector<CPlane> WorldPyrTemp = (static_cast<CClipTrav*>(Trav))->WorldPyramid;
		bool backfaceclipped = false;
		CCluster *pOtherSideCluster;
		if (pPortal->getCluster(0) == pCluster)
			pOtherSideCluster = pPortal->getCluster (1);
		else
			pOtherSideCluster = pPortal->getCluster (0);

		if (pCluster->Father != NULL)
		if (caller == pCluster->Father->getClipObs()) // If the caller is the father
		if (pCluster->VisibleFromFather)
			// Backface clipping
			if( !pPortal->isInFront( (static_cast<CClipTrav*>(Trav))->CamPos ))
				backfaceclipped = true;

		if (!backfaceclipped)
		if (pPortal->clipPyramid ((static_cast<CClipTrav*>(Trav))->CamPos,
								(static_cast<CClipTrav*>(Trav))->WorldPyramid))
		{
			CClusterClipObs *pObserver = pOtherSideCluster->getClipObs();
			pObserver->traverse (this);
		}

		(static_cast<CClipTrav*>(Trav))->WorldPyramid = WorldPyrTemp;
	}

	// Link up in hierarchy
	if ((pCluster->FatherVisible)&&(pCluster->Father != NULL))
	{
		CClusterClipObs *pObserver = pCluster->Father->getClipObs();
		pObserver->traverse (this);
	}

	// Link down in hierarchy
	for (i = 0; i < pCluster->Children.size(); ++i)
	if (pCluster->Children[i]->VisibleFromFather)
	{
		CClusterClipObs *pObserver = pCluster->Children[i]->getClipObs();
		pObserver->traverse (this);
	}

	Visited = false;
}


} // NL3D
